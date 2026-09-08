#include "decoder.h"

#include <cmath>
#include <stdexcept>

#include "spot_stream/MessageHeader.h"
#include "spot_stream/TradesStreamEvent.h"

std::vector<domain::TradeEvent> SBEDecoder::decode_trade(const char* data, std::size_t length) {
    char* buffer = const_cast<char*>(data);

    spot_stream::MessageHeader header(buffer, 0, length, spot_stream::MessageHeader::sbeSchemaVersion());
    if (header.templateId() != spot_stream::TradesStreamEvent::SBE_TEMPLATE_ID) {
        throw std::runtime_error("unexpected SBE templateId: " + std::to_string(header.templateId()));
    }

    spot_stream::TradesStreamEvent event;
    event.wrapForDecode(
        buffer,
        header.encodedLength(),
        header.blockLength(),
        header.version(),
        length);

    const double price_scale = std::pow(10.0, event.priceExponent());
    const double qty_scale = std::pow(10.0, event.qtyExponent());
    const std::int64_t event_time = event.eventTime();
    const std::int64_t transact_time = event.transactTime();

    std::vector<domain::TradeEvent> trades;
    auto& group = event.trades();
    while (group.hasNext()) {
        group.next();
        trades.push_back(domain::TradeEvent{
            event_time,
            transact_time,
            std::string(), // symbol is trailing var-data; filled in below
            group.id(),
            static_cast<double>(group.price()) * price_scale,
            static_cast<double>(group.qty()) * qty_scale,
            group.isBuyerMaker() == spot_stream::BoolEnum::Value::True,
        });
    }

    // Symbol must be read after the group per SBE's sequential-access rule.
    const std::string symbol = event.getSymbolAsString();
    for (auto& trade : trades) {
        trade.symbol = symbol;
    }

    return trades;
}
