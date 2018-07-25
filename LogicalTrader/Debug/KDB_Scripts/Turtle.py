# encoding: utf-8
"""
This is a minimal working version of sample strategy
"""

"""Configuration for Simulation
Under Linux simulation, api in included as part of strategy
by cython, therefore anything from api module should not included.
"""
from  qpython import qconnection
import time as tm
try:
    from my.sdp.api import (Order, Logger, Direction, OpenClose, OrderStatus)
except Exception:
    pass



def on_init(context, config_type, config):
    context.config = config
    context.dic = {}
    context.symboldic = {}
    context.count = 0
    context.time = tm.time()

    context.kindleInterval = 60
    context.volatilityRange = 50
    context.breakRange = 50
    context.NEntryParameter = 0.5
    context.NStoplossPositionParameter = 1.5

    context.PositionClearPrice = 0.0

    context.Nvalue = 0.0
    context.LocalHigh = -999999999.99
    context.LocalLow =  999999999.99
    context.UpDrawBack = 0.0
    context.DownDrawBack = 0.0
    context.kindleNumber = 0

    context.PositionAddedTime = 0
    context.PositionTimesParameter = 4
    context.PositionEntryPrice = {}
    context.LegOnePositionEntryPrice = {}
    context.LegTwoPositionEntryPrice = {}

    context.PairDataBarDate = 0
    context.PairDataBarBidPrice1Close  = 0.0
    context.PairDataBarAskPrice1Close  = 0.0
    context.PairDataBarShortEntry = 0.0
    context.PairDataBarLongEntry  = 0.0
    context.PairDataBarN = 0.0



    context.q = qconnection.QConnection(host='localhost', port=9999)
    context.q.close()
    context.q.open()
    q_initString = """quoteData:([] Date:();`float$LegOneBidPrice1:();`float$LegOneAskPrice1:();`float$LegTwoBidPrice1:();`float$LegTwoAskPrice1:();`float$PairBidPrice:();`float$PairAskPrice:());"""
    context.q.sync(q_initString)


    """    
    Start of trading session

    Parameters
    ----------
    context : :obj:`StrategyWrapper.NoneObject`
        An empty global object for users to carry variables
        across functions. Use of global variables is not
        recommended.

    config_type : {0}
        0 for default configuration type

    config: object

        config
            Strategy configuration structure, when config_type = 0.

        +----------------+----------------------+--------------------------------------+
        | Name           | Type                 | Comment                              |
        +----------------+----------------------+--------------------------------------+
        |trading_date    |integer               |trading date format like YYYYMMDD     |
        +----------------+----------------------+--------------------------------------+
        |day_night       |integer               |0 for day, 1 for night                |
        +----------------+----------------------+--------------------------------------+
        |accounts        |tuple of **account**  |access by ``accounts[<index>]``       |
        +----------------+----------------------+--------------------------------------+
        |contracts       |tuple of **contract** |access by ``contracts[<index>]``      |
        +----------------+----------------------+--------------------------------------+
        |param_file_path |unicode               |external parameter file dir+name      |
        +----------------+----------------------+--------------------------------------+
        |output_file_path|unicode               |temp R/W directory for strategy       |
        +----------------+----------------------+--------------------------------------+
        |st_id           |integer               |Strategy ID                           |
        +----------------+----------------------+--------------------------------------+
        |st_name         |unicode               |Strategy name                         |
        +----------------+----------------------+--------------------------------------+

            account
                account information

            +--------------+-----------+------------------------+
            |Name          |Type       |Comment                 |
            +--------------+-----------+------------------------+
            |account       |unicode    |account name            |
            +--------------+-----------+------------------------+
            |currency      |integer    |cash currency type      |
            +--------------+-----------+------------------------+
            |exch_rate     |double     |currency exchange rate  |
            +--------------+-----------+------------------------+
            |cash_asset    |double     |cash book value         |
            +--------------+-----------+------------------------+
            |cash_available|double     |available cash          |
            +--------------+-----------+------------------------+

            contract
                contract information

            +-------------------+-----------+----------------------------------------------+
            |Name               |Type       |Comment                                       |
            +-------------------+-----------+----------------------------------------------+
            |symbol             |unicode    |e.g. 'a1701' for futures, '600001' for stock  |
            +-------------------+-----------+----------------------------------------------+
            |exch               |integer    |see @enum Exchange                            |
            +-------------------+-----------+----------------------------------------------+
            |max_accum_open_vol |integer    |max vol allowed to open in a single           |
            |                   |           |day                                           |
            +-------------------+-----------+----------------------------------------------+
            |max_cancel_limit   |integer    |max cancel order limit number                 |
            +-------------------+-----------+----------------------------------------------+
            |expiration_date    |integer    |YYYYMMDD e.g. 20170101                        |
            +-------------------+-----------+----------------------------------------------+
            |tick_size          |float      |minimum price unit                            |
            +-------------------+-----------+----------------------------------------------+
            |multiple           |float      |price multiple                                |
            +-------------------+-----------+----------------------------------------------+
            |account            |unicode    |account name                                  |
            +-------------------+-----------+----------------------------------------------+
            |yesterday_pos      |dictionary || KEYS: ``"long_volume"``, ``"long_price"``,  |
            |                   |           || ``"short_volume"``, ``"short_price"``       |
            +-------------------+-----------+----------------------------------------------+
            |today_pos          |dictionary || KEYS: ``"long_volume"``, ``"long_price"``,  |
            |                   |           || ``"short_volume"``, ``"short_price"``       |
            +-------------------+-----------+----------------------------------------------+
            |fee                |dictionary || KEYS: ``"fee_by_lot"``, ``"exchange_fee"``, |
            |                   |           || ``"yes_exchange_fee"``, ``"broker_fee"``,   |
            |                   |           || ``"stamp_tax"``, ``"acc_transfer_fee"``     |
            +-------------------+-----------+----------------------------------------------+


    Returns
    -------
    result : int
        0 for success, -1 for failure.

    """


    context.order = Order(context, config)
    context.logger = Logger(context, config)
    # for simple position management
    context.pos_dict = {}
    for cont in config.contracts:
        context.pos_dict[cont.symbol] = {
            "long_volume": cont.today_pos["long_volume"],
            "short_volume": cont.today_pos["short_volume"]
            }
    context.long_position = lambda x: context.pos_dict[x]["long_volume"]
    context.short_position = lambda x: context.pos_dict[x]["short_volume"]

    context.ORDER_SENT_FLAG = False


def on_book(context, quote_type, quote):
    """
    Receiving tick data and do computations

    Parameters
    ----------
    context : :obj:`StrategyWrapper.NoneObject`
        An empty global object for users to carry variables
        across functions. Use of global variables is not
        recommended.

    quote_type : {0, 1}
        0 for future or spot data, 1 for stock data.

    quote : object

        futures book
            Quote structure for futures and spot, when quote_type = 0.

        +-------------------+-----------+-------------------------------------------+
        |Name               |Type       |Comment                                    |
        +-------------------+-----------+-------------------------------------------+
        |feed_type          |int        |Feed type                                  |
        +-------------------+-----------+-------------------------------------------+
        |symbol             |str        |contract code                              |
        +-------------------+-----------+-------------------------------------------+
        |exchange           |int        |exchange code                              |
        +-------------------+-----------+-------------------------------------------+
        |int_time           |int        |exchange time(HHMMssmmm)                   |
        +-------------------+-----------+-------------------------------------------+
        |pre_close_px       |float      |yesterday close price                      |
        +-------------------+-----------+-------------------------------------------+
        |pre_settle_px      |float      |yesterday settle price                     |
        +-------------------+-----------+-------------------------------------------+
        |pre_open_interest  |float      |yesterday open interest                    |
        +-------------------+-----------+-------------------------------------------+
        |open_interest      |float      |open interest                              |
        +-------------------+-----------+-------------------------------------------+
        |open_px            |float      |open price                                 |
        +-------------------+-----------+-------------------------------------------+
        |high_px            |float      |high price                                 |
        +-------------------+-----------+-------------------------------------------+
        |low_px             |float      |low price                                  |
        +-------------------+-----------+-------------------------------------------+
        |avg_px             |float      |average price                              |
        +-------------------+-----------+-------------------------------------------+
        |last_px            |float      |last price                                 |
        +-------------------+-----------+-------------------------------------------+
        |bp_array           |tuple      |bid price array                            |
        +-------------------+-----------+-------------------------------------------+
        |ap_array           |tuple      |ask price array                            |
        +-------------------+-----------+-------------------------------------------+
        |bv_array           |tuple      |bid volume array                           |
        +-------------------+-----------+-------------------------------------------+
        |av_array           |tuple      |ask volume array                           |
        +-------------------+-----------+-------------------------------------------+
        |total_vol          |int        |total volume traded                        |
        +-------------------+-----------+-------------------------------------------+
        |total_notional     |float      |total notional traded                      |
        +-------------------+-----------+-------------------------------------------+
        |upper_limit_px     |float      |upper limit price                          |
        +-------------------+-----------+-------------------------------------------+
        |lower_limit_px     |float      |lower limit price                          |
        +-------------------+-----------+-------------------------------------------+
        |close_px           |float      |today close price                          |
        +-------------------+-----------+-------------------------------------------+
        |settle_px          |float      |settle price                               |
        +-------------------+-----------+-------------------------------------------+
        |implied_bid_size   |tuple      |implied bid size                           |
        +-------------------+-----------+-------------------------------------------+
        |implied_ask_size   |tuple      |implied ask size                           |
        +-------------------+-----------+-------------------------------------------+
        |total_buy_ordsize  |int        |total buy order size *DCE OrderStat quote* |
        +-------------------+-----------+-------------------------------------------+
        |total_sell_ordsize |int        |total sell order size *DCE OrderStat quote*|
        +-------------------+-----------+-------------------------------------------+
        |weighted_buy_px    |float      |average buy price *DCE OrderStat quote*    |
        +-------------------+-----------+-------------------------------------------+
        |weighted_sell_px   |float      |average sell price *DCE OrderStat quote*   |
        +-------------------+-----------+-------------------------------------------+

        stock book
            Quote structure for stock, when quote_type = 1.

        +------------------+-------+-----------------------------------------------+
        |Name              |Type   |Comment                                        |
        +------------------+-------+-----------------------------------------------+
        |wind_code         |str    |wind_code                                      |
        +------------------+-------+-----------------------------------------------+
        |ticker            |str    |original ticker                                |
        +------------------+-------+-----------------------------------------------+
        |action_day        |int    |date of action(YYYYmmdd)                       |
        +------------------+-------+-----------------------------------------------+
        |trading_day       |int    |current trading date(YYYYmmdd)                 |
        +------------------+-------+-----------------------------------------------+
        |exch_time         |int    |time at exchange(HHMMSSmmm)                    |
        +------------------+-------+-----------------------------------------------+
        |status            |int    |status of stock                                |
        +------------------+-------+-----------------------------------------------+
        |pre_close_px      |int    |yesterday close * 10000                        |
        +------------------+-------+-----------------------------------------------+
        |open_px           |int    |open price * 10000                             |
        +------------------+-------+-----------------------------------------------+
        |high_px           |int    |high price * 10000                             |
        +------------------+-------+-----------------------------------------------+
        |low_px            |int    |low price * 10000                              |
        +------------------+-------+-----------------------------------------------+
        |last_px           |int    |last price * 10000                             |
        +------------------+-------+-----------------------------------------------+
        |ap_array          |tuple  |array of ask price * 10000                     |
        +------------------+-------+-----------------------------------------------+
        |av_array          |tuple  |array of ask volume                            |
        +------------------+-------+-----------------------------------------------+
        |bp_array          |tuple  |array of bid price * 10000                     |
        +------------------+-------+-----------------------------------------------+
        |bv_array          |tuple  |array of bid volume                            |
        +------------------+-------+-----------------------------------------------+
        |num_of_trades     |int    |total number of trades                         |
        +------------------+-------+-----------------------------------------------+
        |total_vol         |int    |total trading volume                           |
        +------------------+-------+-----------------------------------------------+
        |total_notional    |int    |total trading notional                         |
        +------------------+-------+-----------------------------------------------+
        |total_bid_vol     |int    |total bid volume                               |
        +------------------+-------+-----------------------------------------------+
        |total_ask_vol     |int    |total ask volume                               |
        +------------------+-------+-----------------------------------------------+
        |weighted_avg_bp   |int    |weighted average bid price * 10000             |
        +------------------+-------+-----------------------------------------------+
        |weighted_avg_ap   |int    |weighted average ask price * 10000             |
        +------------------+-------+-----------------------------------------------+
        |IOPV              |int    ||Indicative Optimized Portfolio Value * 10000  |
        |                  |       ||(ETF funds)                                   |
        +------------------+-------+-----------------------------------------------+
        |yield_to_maturity |int    |yield to maturity * 10000                      |
        +------------------+-------+-----------------------------------------------+
        |upper_limit_px    |int    |upper limit * 10000                            |
        +------------------+-------+-----------------------------------------------+
        |lower_limit_px    |int    |lower limit * 10000                            |
        +------------------+-------+-----------------------------------------------+
        |prefix            |str    |prefix                                         |
        +------------------+-------+-----------------------------------------------+
        |PE1               |int    |PE ratio(Currently 0)                          |
        +------------------+-------+-----------------------------------------------+
        |PE2               |int    |PE ratio(Currently 0)                          |
        +------------------+-------+-----------------------------------------------+
        |change            |int    |up/down comparing to last tick(Currently 0)    |
        +------------------+-------+-----------------------------------------------+

    """
    date, filterTime = str(context.config.trading_date), int(quote.int_time)
    # print(quote.symbol, quote.int_time)

    if ((filterTime > 93000000) and (filterTime < 113000000)) or (
        (filterTime > 130000000) and (filterTime < 150000000)):
         # print ("Trading Time")
        if str(quote.symbol).__contains__("IH"):
            context.dic["IH"] = [quote.bp_array[0], quote.ap_array[0]]
            context.symboldic["IH"] = quote.symbol
        if str(quote.symbol).__contains__("IC"):
            context.dic["IC"] = [quote.bp_array[0], quote.ap_array[0]]
            context.symboldic["IC"] = quote.symbol
        if len(context.dic.keys()) < 2:
            return
    """
    if len(context.dic.keys()) >= 2:
        sql = "`quoteData insert (%s;%s;%s;%s;%s;%s;%s)"
        time_sql = '{y+ "T"$-9#"00000000",string x}[%s;%s]'
        date_time = time_sql % (filterTime, "%s.%s.%s" % (date[0:4], date[4:6], date[6:8]))
        context.q.sync(date_time)
        # print(context.dic["IC"][0]*200 -context.dic["IH"][1]*300*2)
        feed_quote = sql % (date_time, context.dic["IH"][0], context.dic["IH"][1], context.dic["IC"][0], context.dic["IC"][1], context.dic["IC"][0]*200 -context.dic["IH"][1]*300*2, context.dic["IC"][1]*200 -context.dic["IH"][0]*300*2)
        context.q.sync(feed_quote)

        context.q.sync(
        "CombinedMainContract: select Date:last Date,BidPrice1Open:first PairBidPrice,BidPrice1High:max PairBidPrice,BidPrice1Low:min PairBidPrice, BidPrice1Close:last PairBidPrice,BidVol1:100,AskPrice1Open:first PairAskPrice,AskPrice1High:max PairAskPrice,AskPrice1Low:min PairAskPrice, AskPrice1Close:last PairAskPrice,AskVol1:last 100,LegOneBidPrice1:last LegOneBidPrice1, LegOneAskPrice1:last LegOneAskPrice1, LegTwoBidPrice1:last LegTwoBidPrice1, LegTwoAskPrice1: last LegTwoAskPrice1  by %s xbar Date.second from `quoteData;" % (
        context.kindleInterval))
        context.q.sync(
        "delete date, second from `CombinedMainContract;delete from `CombinedMainContract where Date.second < 09:30:00;delete from `CombinedMainContract where Date.second > 11:30:00, Date.second < 13:00:00;delete from `CombinedMainContract where Date.second > 15:00:00;update TrueRange: {max(x;y;z)}'[(AskPrice1High - BidPrice1Low);(AskPrice1High - (prev BidPrice1Close));((prev AskPrice1High) - BidPrice1Low)]  from `CombinedMainContract;")
        context.q.sync("update N:  mavg[%s;TrueRange] from `CombinedMainContract;" % (context.volatilityRange))
        context.q.sync("update ShortEntry: prev (%s mmin BidPrice1Low), LongEntry: prev (%s mmax AskPrice1High) from `CombinedMainContract;"%(context.breakRange, context.breakRange))

        Signal = context.q.sync("select count Date from CombinedMainContract")[0]
    
        if (Signal[0] > context.kindleNumber):
            context.kindleNumber = Signal[0]
            PairDataBar = context.q.sync("-2#select Date.minute, BidPrice1Close, AskPrice1Close, ShortEntry, LongEntry, N from CombinedMainContract")[0]
            context.PairDataBarDate = PairDataBar[0]
            context.PairDataBarBidPrice1Close = PairDataBar[1]
            context.PairDataBarAskPrice1Close = PairDataBar[2]
            context.PairDataBarShortEntry = PairDataBar[3]
            context.PairDataBarLongEntry = PairDataBar[4]
            context.PairDataBarN = PairDataBar[5]
            if (context.PairDataBarBidPrice1Close < context.LocalLow):
                context.UpDrawBack = 0.0
                context.LocalLow = context.PairDataBarBidPrice1Close
            elif (context.PairDataBarBidPrice1Close > context.LocalLow):
                context.UpDrawBack = context.PairDataBarBidPrice1Close - context.LocalLow

            if (abs(context.PositionAddedTime) > 0 and (context.PairDataBarDate > 898)):
                context.PositionClearPrice = context.dic["IC"][1]
                # print("PosClear: " + str(context.dic["IC"][1]))
                # context.order.send_single_order(context.symboldic["IC"], context.dic["IC"][1], abs(context.PositionAddedTime), Direction.BUY, OpenClose.CLOSE)
                context.PositionAddedTime = 0
                # sendOrderClose PositionAddedTime Amount Contract
                if (context.PositionAddedTime == -1):
                    print(context.LegOnePositionEntryPrice[1] - context.PositionClearPrice)
                elif (context.PositionAddedTime == -2):
                    print(context.LegOnePositionEntryPrice[1] + context.LegOnePositionEntryPrice[
                        2] - 2 * context.PositionClearPrice)
                elif (context.PositionAddedTime == -3):
                    print(context.LegOnePositionEntryPrice[1] + context.LegOnePositionEntryPrice[2] +
                          context.LegOnePositionEntryPrice[3] - 3 * context.PositionClearPrice)
                elif (context.PositionAddedTime == -4):
                    print(context.LegOnePositionEntryPrice[1] + context.LegOnePositionEntryPrice[2] +
                          context.LegOnePositionEntryPrice[3] + context.LegOnePositionEntryPrice[
                              4] - 4 * context.PositionClearPrice)
                context.LegOnePositionEntryPrice = {}
                context.PositionEntryPrice = {}
                # context.order.send_single_order(context.symboldic["IC"], context.dic["IC"][1], abs(context.PositionAddedTime), Direction.BUY,  OpenClose.CLOSE)
                context.PositionAddedTime = 0
            else:
                if ((abs(context.PositionAddedTime) > 0) and (context.UpDrawBack > context.NStoplossPositionParameter * context.PairDataBarN)):
                    # print("PosClear: " + str(context.dic["IC"][1]))
                    context.PositionClearPrice = context.dic["IC"][1]
                    if (context.PositionAddedTime == -1):
                         print (context.LegOnePositionEntryPrice[1] - context.PositionClearPrice)
                    elif (context.PositionAddedTime == -2):
                         print (context.LegOnePositionEntryPrice[1]  + context.LegOnePositionEntryPrice[2]- 2 * context.PositionClearPrice)
                    elif (context.PositionAddedTime == -3):
                         print (context.LegOnePositionEntryPrice[1]  + context.LegOnePositionEntryPrice[2] + context.LegOnePositionEntryPrice[3]- 3 * context.PositionClearPrice)
                    elif (context.PositionAddedTime == -4):
                         print (context.LegOnePositionEntryPrice[1]  + context.LegOnePositionEntryPrice[2] + context.LegOnePositionEntryPrice[3] + context.LegOnePositionEntryPrice[4] - 4 * context.PositionClearPrice)
                    context.LegOnePositionEntryPrice = {}
                    context.PositionEntryPrice = {}
                    #context.order.send_single_order(context.symboldic["IC"], context.dic["IC"][1], abs(context.PositionAddedTime), Direction.BUY,  OpenClose.CLOSE)
                    context.PositionAddedTime = 0
                    context.q.sync("update Position:0 from `CombinedMainContract where Date = max Date")
                if ((abs(context.PositionAddedTime) == 3) and context.PositionTimesParameter >= 4 and context.PairDataBarBidPrice1Close < context.PositionEntryPrice[1] - 3 *  context.NEntryParameter*context.Nvalue):
                    context.PositionAddedTime = -4
                    context.LegOnePositionEntryPrice[4] = context.dic["IC"][0]
                    context.PositionEntryPrice[4] = context.PairDataBarBidPrice1Close
                    # print("Pos4: " + str(context.dic["IC"][0]))
                    #context.order.send_single_order(context.symboldic["IC"], context.dic["IC"][0], 1, Direction.SELL, OpenClose.OPEN)
                    context.q.sync("update Position:-4 from `CombinedMainContract where Date = max Date")
                if ((abs(context.PositionAddedTime) == 2) and context.PositionTimesParameter >= 3 and context.PairDataBarBidPrice1Close < context.PositionEntryPrice[1] - 2 * context.NEntryParameter*context.Nvalue):
                    context.PositionAddedTime = -3
                    context.LegOnePositionEntryPrice[3] = context.dic["IC"][0]
                    context.PositionEntryPrice[3] = context.PairDataBarBidPrice1Close
                    # print("Pos3: " + str(context.dic["IC"][0]))
                    context.q.sync("update Position:-3 from `CombinedMainContract where Date = max Date")
                    #context.order.send_single_order(context.symboldic["IC"], context.dic["IC"][0], 1, Direction.SELL, OpenClose.OPEN)
                if ((abs(context.PositionAddedTime) == 1) and context.PositionTimesParameter >= 2 and context.PairDataBarBidPrice1Close < context.PositionEntryPrice[1] - 1 * context.NEntryParameter*context.Nvalue):
                    context.PositionAddedTime = -2
                    context.LegOnePositionEntryPrice[2] = context.dic["IC"][0]
                    context.PositionEntryPrice[2] = context.PairDataBarBidPrice1Close
                    # print("Pos2: " + str(context.dic["IC"][0]))
                    #context.order.send_single_order(context.symboldic["IC"], context.dic["IC"][0], 1, Direction.SELL, OpenClose.OPEN)
                    context.q.sync("update Position:-2 from `CombinedMainContract where Date = max Date")
                if ((abs(context.PositionAddedTime) == 0) and context.PositionTimesParameter >= 1 and context.PairDataBarBidPrice1Close < context.PairDataBarShortEntry - 0 * context.NEntryParameter*context.Nvalue):
                    context.PositionAddedTime = -1
                    context.LegOnePositionEntryPrice[1] = context.dic["IC"][0]
                    context.PositionEntryPrice[1] = context.PairDataBarBidPrice1Close
                    # print("Pos1: " + str(str(context.dic["IC"][0])))
                    context.Nvalue = context.PairDataBarN
                    context.q.sync("update Position:-1 from `CombinedMainContract where Date = max Date")
                    #context.order.send_single_order(context.symboldic["IC"], context.dic["IC"][0], 1, Direction.SELL, OpenClose.OPEN)

    """
    """
    if context.long_position(quote.symbol) > 0 and not context.ORDER_SENT_FLAG:
        context.order.send_single_order(
            quote.symbol, quote.bp_array[0], 5, Direction.SELL, OpenClose.CLOSE
        )
        context.ORDER_SENT_FLAG = True
    elif 90000000 < quote.int_time < 90500000 and not context.ORDER_SENT_FLAG:
        context.order.send_single_order(
            quote.symbol, quote.bp_array[0], 5, Direction.BUY, OpenClose.OPEN
            )
        context.ORDER_SENT_FLAG = True
    else:
        pass
    """

    if len(context.dic.keys()) >= 2:
        sql = "`quoteData insert (%s;%s;%s;%s;%s;%s;%s)"
        time_sql = '{y+ "T"$-9#"00000000",string x}[%s;%s]'
        date_time = time_sql % (filterTime, "%s.%s.%s" % (date[0:4], date[4:6], date[6:8]))
        context.q.sync(date_time)
        # print(context.dic["IC"][0]*200 -context.dic["IH"][1]*300*2)
        feed_quote = sql % (
        date_time, context.dic["IH"][0], context.dic["IH"][1], context.dic["IC"][0], context.dic["IC"][1],
        context.dic["IC"][0] * 200 - context.dic["IH"][1] * 300 * 2,
        context.dic["IC"][1] * 200 - context.dic["IH"][0] * 300 * 2)
        context.q.sync(feed_quote)

        context.q.sync(
            "CombinedMainContract: select Date:last Date,BidPrice1Open:first PairBidPrice,BidPrice1High:max PairBidPrice,BidPrice1Low:min PairBidPrice, BidPrice1Close:last PairBidPrice,BidVol1:100,AskPrice1Open:first PairAskPrice,AskPrice1High:max PairAskPrice,AskPrice1Low:min PairAskPrice, AskPrice1Close:last PairAskPrice,AskVol1:last 100,LegOneBidPrice1:last LegOneBidPrice1, LegOneAskPrice1:last LegOneAskPrice1, LegTwoBidPrice1:last LegTwoBidPrice1, LegTwoAskPrice1: last LegTwoAskPrice1  by %s xbar Date.second from `quoteData;" % (
                context.kindleInterval))
        context.q.sync(
            "delete date, second from `CombinedMainContract;delete from `CombinedMainContract where Date.second < 09:30:00;delete from `CombinedMainContract where Date.second > 11:30:00, Date.second < 13:00:00;delete from `CombinedMainContract where Date.second > 15:00:00;update TrueRange: {max(x;y;z)}'[(AskPrice1High - BidPrice1Low);(AskPrice1High - (prev BidPrice1Close));((prev AskPrice1High) - BidPrice1Low)]  from `CombinedMainContract;")
        context.q.sync("update N:  mavg[%s;TrueRange] from `CombinedMainContract;" % (context.volatilityRange))
        context.q.sync(
            "update ShortEntry: prev (%s mmin BidPrice1Low), LongEntry: prev (%s mmax AskPrice1High) from `CombinedMainContract;" % (
            context.breakRange, context.breakRange))

        Signal = context.q.sync("select count Date from CombinedMainContract")[0]

        if (Signal[0] > context.kindleNumber):
            context.kindleNumber = Signal[0]
            PairDataBar = context.q.sync(
                "-2#select Date.minute, BidPrice1Close, AskPrice1Close, ShortEntry, LongEntry, N from CombinedMainContract")[
                0]
            context.PairDataBarDate = PairDataBar[0]
            context.PairDataBarBidPrice1Close = PairDataBar[1]
            context.PairDataBarAskPrice1Close = PairDataBar[2]
            context.PairDataBarShortEntry = PairDataBar[3]
            context.PairDataBarLongEntry = PairDataBar[4]
            context.PairDataBarN = PairDataBar[5]
            if (context.PairDataBarAskPrice1Close > context.LocalHigh):
                context.DownDrawBack = 0.0
                context.LocalHigh = context.PairDataBarAskPrice1Close
            elif (context.PairDataBarAskPrice1Close < context.LocalHigh):
                context.DownDrawBack = context.LocalHigh - context.PairDataBarAskPrice1Close

            if (abs(context.PositionAddedTime) > 0 and (context.PairDataBarDate > 898)):
                context.PositionClearPrice = context.dic["IC"][0]
                # print("PosClear: " + str(context.dic["IC"][1]))
                # context.order.send_single_order(context.symboldic["IC"], context.dic["IC"][1], abs(context.PositionAddedTime), Direction.BUY, OpenClose.CLOSE)

                # sendOrderClose PositionAddedTime Amount Contract
                print("PosClear: " + str(context.dic["IC"][0]))
                context.PositionClearPrice = context.dic["IC"][0]
                if (context.PositionAddedTime == 1):
                    print(-(context.LegOnePositionEntryPrice[1] - context.PositionClearPrice))
                elif (context.PositionAddedTime == 2):
                    print(-(context.LegOnePositionEntryPrice[1] + context.LegOnePositionEntryPrice[
                        2] - 2 * context.PositionClearPrice))
                elif (context.PositionAddedTime == 3):
                    print(-(context.LegOnePositionEntryPrice[1] + context.LegOnePositionEntryPrice[2] +
                            context.LegOnePositionEntryPrice[3] - 3 * context.PositionClearPrice))
                elif (context.PositionAddedTime == 4):
                    print(-(context.LegOnePositionEntryPrice[1] + context.LegOnePositionEntryPrice[2] +
                            context.LegOnePositionEntryPrice[3] + context.LegOnePositionEntryPrice[
                                4] - 4 * context.PositionClearPrice))
                context.PositionAddedTime = 0
                context.LegOnePositionEntryPrice = {}
                context.PositionEntryPrice = {}
                # context.order.send_single_order(context.symboldic["IC"], context.dic["IC"][1], abs(context.PositionAddedTime), Direction.BUY,  OpenClose.CLOSE)
                context.PositionAddedTime = 0
            else:
                if ((abs(context.PositionAddedTime) > 0) and (
                    context.DownDrawBack > context.NStoplossPositionParameter * context.PairDataBarN)):
                    print("PosClear: " + str(context.dic["IC"][0]))
                    context.PositionClearPrice = context.dic["IC"][0]
                    if (context.PositionAddedTime == 1):
                        print(-(context.LegOnePositionEntryPrice[1] - context.PositionClearPrice))
                    elif (context.PositionAddedTime == 2):
                        print(-(context.LegOnePositionEntryPrice[1] + context.LegOnePositionEntryPrice[
                            2] - 2 * context.PositionClearPrice))
                    elif (context.PositionAddedTime == 3):
                        print(-(context.LegOnePositionEntryPrice[1] + context.LegOnePositionEntryPrice[2] +
                              context.LegOnePositionEntryPrice[3] - 3 * context.PositionClearPrice))
                    elif (context.PositionAddedTime == 4):
                        print(-(context.LegOnePositionEntryPrice[1] + context.LegOnePositionEntryPrice[2] +
                              context.LegOnePositionEntryPrice[3] + context.LegOnePositionEntryPrice[
                                  4] - 4 * context.PositionClearPrice))
                    context.LegOnePositionEntryPrice = {}
                    context.PositionEntryPrice = {}
                    # context.order.send_single_order(context.symboldic["IC"], context.dic["IC"][1], abs(context.PositionAddedTime), Direction.BUY,  OpenClose.CLOSE)
                    context.PositionAddedTime = 0
                    context.q.sync("update Position:0 from `CombinedMainContract where Date = max Date")
                if ((abs(
                        context.PositionAddedTime) == 3) and context.PositionTimesParameter >= 4 and context.PairDataBarAskPrice1Close >
                        context.PositionEntryPrice[1] + 3 * context.NEntryParameter * context.Nvalue):
                    context.PositionAddedTime = 4
                    context.LegOnePositionEntryPrice[4] = context.dic["IC"][1]
                    context.PositionEntryPrice[4] = context.PairDataBarAskPrice1Close
                    print("Pos4: " + str(context.dic["IC"][1]))
                    # context.order.send_single_order(context.symboldic["IC"], context.dic["IC"][1], 1, Direction.SELL, OpenClose.OPEN)
                    context.q.sync("update Position:4 from `CombinedMainContract where Date = max Date")
                if ((abs(
                        context.PositionAddedTime) == 2) and context.PositionTimesParameter >= 3 and context.PairDataBarAskPrice1Close >
                        context.PositionEntryPrice[1] + 2 * context.NEntryParameter * context.Nvalue):
                    context.PositionAddedTime = 3
                    context.LegOnePositionEntryPrice[3] = context.dic["IC"][1]
                    context.PositionEntryPrice[3] = context.PairDataBarAskPrice1Close
                    print("Pos3: " + str(context.dic["IC"][1]))
                    context.q.sync("update Position:3 from `CombinedMainContract where Date = max Date")
                    # context.order.send_single_order(context.symboldic["IC"], context.dic["IC"][1], 1, Direction.SELL, OpenClose.OPEN)
                if ((abs(
                        context.PositionAddedTime) == 1) and context.PositionTimesParameter >= 2 and context.PairDataBarAskPrice1Close >
                        context.PositionEntryPrice[1] + 1 * context.NEntryParameter * context.Nvalue):
                    context.PositionAddedTime = 2
                    context.LegOnePositionEntryPrice[2] = context.dic["IC"][1]
                    context.PositionEntryPrice[2] = context.PairDataBarAskPrice1Close
                    print("Pos2: " + str(context.dic["IC"][1]))
                    # context.order.send_single_order(context.symboldic["IC"], context.dic["IC"][1], 1, Direction.SELL, OpenClose.OPEN)
                    context.q.sync("update Position:2 from `CombinedMainContract where Date = max Date")
                if ((abs(
                        context.PositionAddedTime) == 0) and context.PositionTimesParameter >= 1 and context.PairDataBarAskPrice1Close > context.PairDataBarLongEntry + 0 * context.NEntryParameter * context.Nvalue):
                    context.PositionAddedTime = 1
                    context.LegOnePositionEntryPrice[1] = context.dic["IC"][1]
                    context.PositionEntryPrice[1] = context.PairDataBarAskPrice1Close
                    print("Pos1: " + str(str(context.dic["IC"][1])))
                    context.Nvalue = context.PairDataBarN
                    context.q.sync("update Position:1 from `CombinedMainContract where Date = max Date")
                    # context.order.send_single_order(context.symboldic["IC"], context.dic["IC"][1], 1, Direction.SELL, OpenClose.OPEN)







def on_response(context, response_type, response):
    """
    Receiving information on trade results

    Parameters
    ----------
    context : :obj:`StrategyWrapper.NoneObject`
        An empty global object for users to carry variables
        across functions. Use of global variables is not
        recommended.

    response_type: {0}
        0 for default response type

    response: object

        response
            Response structure, when response_type = 0.

        +---------------+-----------+---------------------------+
        |Name           |Type       |Comment                    |
        +---------------+-----------+---------------------------+
        |order_id       |int        |Order ID                   |
        +---------------+-----------+---------------------------+
        |symbol         |unicode    |contract symbol            |
        +---------------+-----------+---------------------------+
        |direction      |int        |0 for buy, 1 for sell      |
        +---------------+-----------+---------------------------+
        |open_close     |int        || 0 for open, 1 for close, |
        |               |           || 2 for close today,       |
        |               |           || 3 for close yesterday    |
        +---------------+-----------+---------------------------+
        |exe_price      |double     |price executed             |
        +---------------+-----------+---------------------------+
        |exe_volume     |int        |volume executed            |
        +---------------+-----------+---------------------------+
        |status         |int        |response type              |
        +---------------+-----------+---------------------------+
        |error_no       |int        |error code                 |
        +---------------+-----------+---------------------------+
        |error_info     |unicode    |error message              |
        +---------------+-----------+---------------------------+

    """
    if response.status in (OrderStatus.SUCCEED.value, OrderStatus.PARTED.value) and response_type == 0:
        if response.exe_volume == 0:
            return
        if response.direction == Direction.BUY.value:
            if response.open_close == OpenClose.OPEN.value:
                context.pos_dict[response.symbol]['long_volume'] += response.exe_volume
            elif response.open_close in (OpenClose.CLOSE.value, OpenClose.CLOSE_YES.value):
                context.pos_dict[response.symbol]['short_volume'] -= response.exe_volume
        elif response.direction == Direction.SELL.value:
            if response.open_close == OpenClose.OPEN.value:
                context.pos_dict[response.symbol]['short_volume'] += response.exe_volume
            elif response.open_close in (OpenClose.CLOSE.value, OpenClose.CLOSE_YES.value):
                context.pos_dict[response.symbol]['long_volume'] -= response.exe_volume


def on_timer(context, data_type, data):
    """
    A Timer function with preset timer interval.

    Parameters
    ----------
    context : :obj:`StrategyWrapper.NoneObject`
        An empty global object for users to carry variables
        across functions. Use of global variables is not
        recommended.

    data_type: int
        Currently not in use.

    data : object
        Customized data structure for future use.(currently not in use)

    """
    pass


def on_session_finish(context):
    """
    Call at the end of each trading session, clear all variables

    Parameters
    ----------
    context : :obj:`StrategyWrapper.NoneObject`
        An empty global object for users to carry variables
        across functions. Use of global variables is not
        recommended.

    """
    pass


if __name__ == "__main__":
    import os
    from my.sdp.simu.config import StratConfig
    from my.sdp.simu.simulator import execute_task

    # set the strategy name and strategy id
    st1 = StratConfig.StratItem(strat_name="test", strat_id=1)
    # product|rank|exchange|mi_type|max_pos|data_source(0)|account
    st1.contracts = [
        "IC|R1|CFFEX|12|1|0|Account1",
        "IH|R1|CFFEX|12|1|0|Account1"
    ]
    #st1.contracts = ["IH|R1|CFFEX|12|1|0|Account1"]

    # account|available_cash|asset_cash|currency|exchange_ratio
    st1.accounts = ["Account1|999999.99|888888.88|CNY|1.0"]

    cfg1 = StratConfig()
    cfg1.start_date = 20161201
    cfg1.end_date = 20161230
    cfg1.day_night_flag = 0
    cfg1.strat_item = st1.items()
    cfg1.strategy = os.path.basename(__file__)
    execute_task(cfg1.totask())