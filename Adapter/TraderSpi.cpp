#include "stdafx.h"

#include <iostream>
using namespace std;

#include ".\ThostTraderApi\ThostFtdcTraderApi.h"
#include "TraderSpi.h"

#pragma warning(disable : 4996)

namespace CTP{

	// 会话参数
	TThostFtdcFrontIDType	FRONT_ID;	//前置编号
	TThostFtdcSessionIDType	SESSION_ID;	//会话编号
	TThostFtdcOrderRefType	ORDER_REF;	//报单引用

	void CTraderSpi::OnFrontConnected()
	{
		cerr << "--->>> " << __FUNCTION__ << endl;
		///用户登录请求
		ReqUserLogin();
		//ctpTraderConfig
		if (ctpTraderConfig->ctpTraderSpi!=nullptr)
		{
			ctpTraderConfig->ctpTraderSpi->OnFrontConnected();
		}
	}

	void CTraderSpi::ReqUserLogin()
	{
		CThostFtdcReqUserLoginField req;
		memset(&req, 0, sizeof(req));
		strcpy(req.BrokerID, BROKER_ID);
		strcpy(req.UserID, INVESTOR_ID);
		strcpy(req.Password, PASSWORD);
		int iResult = pUserApi->ReqUserLogin(&req, ++iRequestID);
		cerr << "--->>> 发送用户登录请求: " << ((iResult == 0) ? "成功" : "失败") << endl;
	}

	void CTraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		cerr << "--->>> " << __FUNCTION__ << endl;
		if (bIsLast && !IsErrorRspInfo(pRspInfo))
		{
			// 保存会话参数
			FRONT_ID = pRspUserLogin->FrontID;
			SESSION_ID = pRspUserLogin->SessionID;
			int iNextOrderRef = atoi(pRspUserLogin->MaxOrderRef);
			iNextOrderRef++;
			sprintf(ORDER_REF, "%d", iNextOrderRef);
			///获取当前交易日
			cerr << "--->>> 获取当前交易日 = " << pUserApi->GetTradingDay() << endl;
			///投资者结算结果确认
			ReqSettlementInfoConfirm();
		}
	}

	void CTraderSpi::ReqSettlementInfoConfirm()
	{
		CThostFtdcSettlementInfoConfirmField req;
		memset(&req, 0, sizeof(req));
		strcpy(req.BrokerID, BROKER_ID);
		strcpy(req.InvestorID, INVESTOR_ID);
		int iResult = pUserApi->ReqSettlementInfoConfirm(&req, ++iRequestID);
		cerr << "--->>> 投资者结算结果确认: " << ((iResult == 0) ? "成功" : "失败") << endl;
	}

	void CTraderSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		cerr << "--->>> " << __FUNCTION__ << endl;
		if (bIsLast && !IsErrorRspInfo(pRspInfo))
		{
			///请求查询合约
			ReqQryInstrument();
		}
	}

	void CTraderSpi::ReqQryInstrument()
	{
		CThostFtdcQryInstrumentField req;
		memset(&req, 0, sizeof(req));
		strcpy(req.InstrumentID, INSTRUMENT_ID);
		int iResult = pUserApi->ReqQryInstrument(&req, ++iRequestID);
		cerr << "--->>> 请求查询合约: " << ((iResult == 0) ? "成功" : "失败") << endl;
	}

	void CTraderSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		cerr << "--->>> " << __FUNCTION__ << endl;
		if (bIsLast && !IsErrorRspInfo(pRspInfo))
		{
			///请求查询合约
			ReqQryTradingAccount();
		}

	}

	void CTraderSpi::ReqQryTradingAccount()
	{
		CThostFtdcQryTradingAccountField req;
		memset(&req, 0, sizeof(req));	
		strcpy(req.BrokerID, BROKER_ID);
		strcpy(req.InvestorID, INVESTOR_ID);
		int iResult = pUserApi->ReqQryTradingAccount(&req, ++iRequestID);
		cerr << "--->>> 请求查询资金账户: " << ((iResult == 0) ? "成功" : "失败") << iResult << endl;
	}

	void CTraderSpi::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		cerr << "--->>> " << __FUNCTION__ << endl;
		if (bIsLast && !IsErrorRspInfo(pRspInfo))
		{
			///请求查询投资者持仓
			ReqQryInvestorPosition();
		}
	}

	void CTraderSpi::ReqQryInvestorPosition()
	{
		CThostFtdcQryInvestorPositionField req;
		memset(&req, 0, sizeof(req));
		strcpy(req.BrokerID, BROKER_ID);
		strcpy(req.InvestorID, INVESTOR_ID);
		strcpy(req.InstrumentID, INSTRUMENT_ID);
		int iResult = pUserApi->ReqQryInvestorPosition(&req, ++iRequestID);
		cerr << "--->>> 请求查询投资者持仓: " << ((iResult == 0) ? "成功" : "失败") << endl;
	}

	void CTraderSpi::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		cerr << "--->>> " << __FUNCTION__ << endl;
		if (bIsLast && !IsErrorRspInfo(pRspInfo))
		{
			///报单录入请求
			ReqOrderInsert();
		}
	}

	void CTraderSpi::ReqOrderInsert()
	{
		CThostFtdcInputOrderField req;
		memset(&req, 0, sizeof(req));
		///经纪公司代码
		strcpy(req.BrokerID, BROKER_ID);
		///投资者代码
		strcpy(req.InvestorID, INVESTOR_ID);
		///合约代码
		strcpy(req.InstrumentID, INSTRUMENT_ID);
		///报单引用
		strcpy(req.OrderRef, ORDER_REF);
		///用户代码
		//	TThostFtdcUserIDType	UserID;
		///报单价格条件: 限价
		req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
		///买卖方向: 
		req.Direction = DIRECTION;
		///组合开平标志: 开仓
		req.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
		///组合投机套保标志
		req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
		///价格
		req.LimitPrice = LIMIT_PRICE;
		///数量: 1
		req.VolumeTotalOriginal = 1;
		///有效期类型: 当日有效
		req.TimeCondition = THOST_FTDC_TC_GFD;
		///GTD日期
		//	TThostFtdcDateType	GTDDate;
		///成交量类型: 任何数量
		req.VolumeCondition = THOST_FTDC_VC_AV;
		///最小成交量: 1
		req.MinVolume = 1;
		///触发条件: 立即
		req.ContingentCondition = THOST_FTDC_CC_Immediately;
		///止损价
		//	TThostFtdcPriceType	StopPrice;
		///强平原因: 非强平
		req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
		///自动挂起标志: 是
		req.IsAutoSuspend = 1;
		///业务单元
		//	TThostFtdcBusinessUnitType	BusinessUnit;
		///请求编号
		//	TThostFtdcRequestIDType	RequestID;
		///用户强评标志: 否
		req.UserForceClose = 0;

		int iResult = pUserApi->ReqOrderInsert(&req, ++iRequestID);
		cerr << "--->>> 报单录入请求: " << ((iResult == 0) ? "成功" : "失败") << endl;
	}

	void CTraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		cerr << "--->>> " << __FUNCTION__ << endl;
		IsErrorRspInfo(pRspInfo);
	}

	void CTraderSpi::ReqOrderAction(CThostFtdcOrderField *pOrder)
	{
		static bool ORDER_ACTION_SENT = false;		//是否发送了报单
		if (ORDER_ACTION_SENT)
			return;

		CThostFtdcInputOrderActionField req;
		memset(&req, 0, sizeof(req));
		///经纪公司代码
		strcpy(req.BrokerID, pOrder->BrokerID);
		///投资者代码
		strcpy(req.InvestorID, pOrder->InvestorID);
		///报单操作引用
		//	TThostFtdcOrderActionRefType	OrderActionRef;
		///报单引用
		strcpy(req.OrderRef, pOrder->OrderRef);
		///请求编号
		//	TThostFtdcRequestIDType	RequestID;
		///前置编号
		req.FrontID = FRONT_ID;
		///会话编号
		req.SessionID = SESSION_ID;
		///交易所代码
		//	TThostFtdcExchangeIDType	ExchangeID;
		///报单编号
		//	TThostFtdcOrderSysIDType	OrderSysID;
		///操作标志
		req.ActionFlag = THOST_FTDC_AF_Delete;
		///价格
		//	TThostFtdcPriceType	LimitPrice;
		///数量变化
		//	TThostFtdcVolumeType	VolumeChange;
		///用户代码
		//	TThostFtdcUserIDType	UserID;
		///合约代码
		strcpy(req.InstrumentID, pOrder->InstrumentID);

		int iResult = pUserApi->ReqOrderAction(&req, ++iRequestID);
		cerr << "--->>> 报单操作请求: " << ((iResult == 0) ? "成功" : "失败") << endl;
		ORDER_ACTION_SENT = true;
	}

	void CTraderSpi::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		cerr << "--->>> " << __FUNCTION__ << endl;
		IsErrorRspInfo(pRspInfo);
	}

	///报单通知
	void CTraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
	{
		cerr << "--->>> " << __FUNCTION__  << endl;
		if (IsMyOrder(pOrder))
		{
			if (IsTradingOrder(pOrder))
				ReqOrderAction(pOrder);
			else if (pOrder->OrderStatus == THOST_FTDC_OST_Canceled)
				cout << "--->>> 撤单成功" << endl;
		}
		if (ctpTraderConfig->ctpTraderSpi!=nullptr)
		{
			CSCThostFtdcOrderField cspOrder;
			ctpTraderConfig->ctpTraderSpi->OnRtnOrder(%cspOrder);
		}
	}

	///成交通知
	void CTraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
	{
		cerr << "--->>> " << __FUNCTION__  << endl;
		if (ctpTraderConfig->ctpTraderSpi!=nullptr)
		{
			CSCThostFtdcTradeField cspTrader;
		
			ctpTraderConfig->ctpTraderSpi->OnRtnTrade(%cspTrader);
		}
	}

	void CTraderSpi:: OnFrontDisconnected(int nReason)
	{
		cerr << "--->>> " << __FUNCTION__ << endl;
		cerr << "--->>> Reason = " << nReason << endl;
		if (ctpTraderConfig->ctpTraderSpi!=nullptr)
			ctpTraderConfig->ctpTraderSpi->OnFrontDisconnected(nReason);
	}

	void CTraderSpi::OnHeartBeatWarning(int nTimeLapse)
	{
		cerr << "--->>> " << __FUNCTION__ << endl;
		cerr << "--->>> nTimerLapse = " << nTimeLapse << endl;
		if (ctpTraderConfig->ctpTraderSpi!=nullptr)
			ctpTraderConfig->ctpTraderSpi->OnHeartBeatWarning(nTimeLapse);
	}

	void CTraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		cerr << "--->>> " << __FUNCTION__ << endl;
		IsErrorRspInfo(pRspInfo);
		if (ctpTraderConfig->ctpTraderSpi!=nullptr)
		{
			CSCThostFtdcRspInfoField cspRspInfo;
			cspRspInfo.ErrorID=pRspInfo->ErrorID;
			cspRspInfo.ErrorMsg=gcnew  String(pRspInfo->ErrorMsg);
			ctpTraderConfig->ctpTraderSpi->OnRspError(%cspRspInfo, nRequestID ,bIsLast);
		}
	}

	bool CTraderSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
	{
		// 如果ErrorID != 0, 说明收到了错误的响应
		bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
		if (bResult)
			cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << endl;
		return bResult;
	}

	bool CTraderSpi::IsMyOrder(CThostFtdcOrderField *pOrder)
	{
		return ((pOrder->FrontID == FRONT_ID) &&
			(pOrder->SessionID == SESSION_ID) &&
			(strcmp(pOrder->OrderRef, ORDER_REF) == 0));
	}

	bool CTraderSpi::IsTradingOrder(CThostFtdcOrderField *pOrder)
	{
		return ((pOrder->OrderStatus != THOST_FTDC_OST_PartTradedNotQueueing) &&
			(pOrder->OrderStatus != THOST_FTDC_OST_Canceled) &&
			(pOrder->OrderStatus != THOST_FTDC_OST_AllTraded));
	}

}