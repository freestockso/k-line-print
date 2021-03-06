#include "StdAfx.h"
#include "Utility.h"
#include "KLineCollection.h"

KLineCollection::KLineCollection(void)
{
	m_nMaxPrice = m_nMinPrice = 0;
}

KLineCollection::~KLineCollection(void)
{

}

KLine KLineCollection::GetKLineByTime(int nTime)
{
	if(m_mapTime2Idx.find(nTime) == m_mapTime2Idx.end())
	{
		KLine tmp;
		memset(&tmp, 0, sizeof(tmp));
		return tmp;
	}
	else
	{
		return (*this)[m_mapTime2Idx[nTime]];
	}
}

//	统一接口用于创建索引
void KLineCollection::AddToTail(KLine kline)
{
	m_mapTime2Idx[kline.time] = this->size();
	push_back(kline);
}

void KLineCollection::Clear()
{
	clear();
	m_mapTime2Idx.clear();
	m_mapKeyPrice.clear();
	m_nKLinePeriod = 0;
	m_nMaxPrice = m_nMinPrice = 0;
}

void KLineCollection::GetPriceVolRange(int nStartIdx, int nEndIdx, int& nHighPr, int& nLowPr, 
									   int& nMaxVol, int& nMaxInterest, float& fHiMacd, float& fLoMacd)
{
	int high = 0, low = 0, maxvol = 0, maxint = 5000;

	float hiMacd = 0, loMacd = 0;

	for(int i = nStartIdx; i <= nEndIdx; i++)
	{
		if(i >= this->size()) continue;

		KLine kline = (*this)[i];
		
		if(i == nStartIdx)
		{
			high = kline.high;
			low = kline.low;
			maxvol = kline.vol;
			hiMacd = kline.MACD;
			loMacd = kline.MACD;
		}
		else
		{
			if(kline.high > high) high = kline.high;
			if(kline.low < low) low = kline.low;
			if(kline.vol > maxvol) maxvol = kline.vol;
			if(abs(kline.interest) > maxint) maxint = abs(kline.interest);
			if(kline.MACD > hiMacd) hiMacd = kline.MACD;
			if(kline.MACD < loMacd) loMacd = kline.MACD;
		}
	}

	nHighPr = high;
	nLowPr = low;
	nMaxVol = maxvol;
	nMaxInterest = maxint;
	fHiMacd = hiMacd;
	fLoMacd = loMacd;
}

//	开始接收分笔数据
void KLineCollection::StartQuote(Tick tick)
{
	KLine tmp;

	tmp.high = tmp.low = tmp.open = tmp.close = tick.price;
	tmp.vol = tick.vol;

	tmp.time = tick.time_ms;
	tmp.start_time = tick.time_ms;

	tmp.vol_acc = tick.vol;
	tmp.price_acc = tick.vol * tick.price;

	m_nMaxPrice = m_nMinPrice = tick.price;
	tmp.interest = 0;

	//	计算初始macd
	tmp.EMA12 = tmp.EMA26 = tick.price;
	tmp.MACD = tmp.EMA12 - tmp.EMA26;
	tmp.avgMACD9 = tmp.MACD;
	tmp.MACDDiff = tmp.MACD - tmp.avgMACD9;

	AddToTail(tmp);
}

int KLineCollection::GetAvgDevi(KLine kline)
{
	if(abs(kline.high - kline.avg) > abs(kline.low - kline.avg))
		return abs(kline.high - kline.avg);
	else
		return abs(kline.low - kline.avg);
}

double sFactor12 = 2.0f / (1+12);
double sFactor26 = 2.0f / (1+26);
double sFactor9 = 2.0f / (1+9);

//	接收分笔数据
void KLineCollection::Quote(Tick tick)
{
	KLine& curKLine = (*this)[this->size() - 1];

	if(tick.price > m_nMaxPrice) m_nMaxPrice = tick.price;

	if(tick.price < m_nMinPrice) m_nMinPrice = tick.price;

	if((tick.time_ms / m_nKLinePeriod) != (curKLine.start_time / m_nKLinePeriod))
	{
		curKLine.time = tick.time_ms;
		curKLine.avg = curKLine.price_acc / (float) curKLine.vol_acc;
		curKLine.avg_devi = GetAvgDevi(curKLine);
		curKLine.interest += tick.interest;

		/* 新起K线 */
		KLine tmp;
		
		tmp.high = tmp.low = tmp.open = tmp.close = tick.price;
		tmp.vol = tick.vol;
		
		tmp.time = tick.time_ms;
		tmp.start_time = tick.time_ms;

		tmp.vol_acc = curKLine.vol_acc + tick.vol;
		tmp.price_acc = curKLine.price_acc + tick.vol * tick.price;
		tmp.avg = tmp.price_acc / (float) tmp.vol_acc;
		tmp.avg_devi = GetAvgDevi(tmp);
		tmp.interest = curKLine.interest;

		float nSumOfClose = 0;

		//	计算ma20
		if(size() >= 19)
		{

			for(int i = size() - 1; i >= (int)size() - 19; i--)
			{
				KLine& tmp1 = (*this)[i];
				nSumOfClose += tmp1.close;
			}

			tmp.sumOf19 = nSumOfClose;
		}

		if(size() >= 59)
		{
			for(int i = size() - 20; i >= (int)size() - 59; i--)
			{
				KLine& tmp1 = (*this)[i];
				nSumOfClose += tmp1.close;
			}

			tmp.sumOf59 = nSumOfClose;
		}

		//	计算macd(12,26,9)
		tmp.EMA12 = curKLine.EMA12 + sFactor12 * (tmp.close - curKLine.EMA12);
		tmp.EMA26 = curKLine.EMA26 + sFactor26 * (tmp.close - curKLine.EMA26);
		tmp.MACD = tmp.EMA12 - tmp.EMA26;
		tmp.avgMACD9 = curKLine.avgMACD9 + sFactor9 * (tmp.MACD - curKLine.avgMACD9);
		tmp.MACDDiff = tmp.MACD - tmp.avgMACD9;

		AddToTail(tmp);
	}
	else
	{
		if(tick.price > curKLine.high) 
			curKLine.high = tick.price;

		if(tick.price < curKLine.low) 
			curKLine.low = tick.price;

		curKLine.time = tick.time_ms;
		curKLine.close = tick.price;
		curKLine.vol += tick.vol;
		curKLine.vol_acc += tick.vol;
		curKLine.price_acc += tick.vol * tick.price;
		curKLine.avg = curKLine.price_acc / (float) curKLine.vol_acc;
		curKLine.avg_devi = GetAvgDevi(curKLine);
		curKLine.interest += tick.interest;

		//	计算ma20
		curKLine.ma20 = (curKLine.sumOf19 + curKLine.close) / 20.0f;
		curKLine.ma60 = (curKLine.sumOf59 + curKLine.close) / 60.0f;

		if(size() == 1)
		{
			curKLine.EMA12 = curKLine.EMA26 = curKLine.close;
			curKLine.MACD = curKLine.EMA12 - curKLine.EMA26;
			curKLine.avgMACD9 = curKLine.MACD;
			curKLine.MACDDiff = curKLine.MACD - curKLine.avgMACD9;
		}
		else
		{
			KLine& lastKLine = (*this)[this->size() - 2];
			curKLine.EMA12 = lastKLine.EMA12 + sFactor12 * (curKLine.close - lastKLine.EMA12);
			curKLine.EMA26 = lastKLine.EMA26 + sFactor26 * (curKLine.close - lastKLine.EMA26);
			curKLine.MACD = curKLine.EMA12 - curKLine.EMA26;
			curKLine.avgMACD9 = lastKLine.avgMACD9 + sFactor9 * (curKLine.MACD - lastKLine.avgMACD9);
			curKLine.MACDDiff = curKLine.MACD - curKLine.avgMACD9;
		}
	}
}

void KLineCollection::Generate(TickCollection& ticks, int seconds)
{
	int nLastSecond;

	/* 分钟K线的数值 */
	int kOpen, kClose, kHigh, kLow, kVol;
	float totalPrice = 0, totalVol = 0, kCount = 0;

	kOpen = kClose = kHigh = kLow = ticks[0].price;
	nLastSecond = ticks[0].time_ms;
	kVol = ticks[0].vol;
	totalVol = kVol;
	totalPrice = kClose * kVol;

	// 首K线是前日的日K，不能计算进总成交量

	for(int i = 1; i < ticks.size(); i++)
	{
		int nCurSecond = ticks[i].time_ms;
		int price = ticks[i].price;
		int vol = ticks[i].vol;

		totalPrice += (price*vol);
		totalVol += vol;

		if((nCurSecond / seconds) != (nLastSecond / seconds))
		{
			/* K 已完成，写入文件 */ 
			KLine kline;

			kline.time = Utility::ConvContTimeToDispTime(nCurSecond);
			kline.high = kHigh;
			kline.low = kLow;
			kline.open = kOpen;
			kline.close = kClose;
			kline.vol = kVol;
			kline.vol_acc = totalVol;

			//	计算均价线
			kline.avg = totalPrice / totalVol;

			AddToTail(kline);

			kCount++;

			/* 新起K线 */
			kOpen = kClose = kHigh = kLow = price;
			kVol = vol;
		}
		else
		{
			if(price > kHigh) kHigh = price;
			if(price < kLow) kLow = price;
			kClose = price;
			kVol += vol;
		}

		nLastSecond = nCurSecond;		
	}
}

void KLineCollection::Generate(TickCollection& ticks, int seconds, KLine prevDayLine)
{
	/* 昨天本合约的日K */
	AddToTail(prevDayLine);
	Generate(ticks, seconds);
}
