#include "ydrcreatewsreport.h"

namespace ydd
{
    YdrCreateWsReport::YdrCreateWsReport(std::string& token, Phrases& phrases, 
	    GeoId& geoId, boost::asio::io_service& ios, bool useSandbox,
	    YdProcess::Callback ydProcessCallback) :
	YdRequest(token, ios, useSandbox, ydProcessCallback),
	phrases_(phrases),
	geoId_(geoId)
    {
    }

    void YdrCreateWsReport::generateRequest()
    {
	using namespace boost::property_tree;
	ptRequest_.put("method", "CreateNewWordstatReport");
	ptRequest_.put("locale", locale_);
	ptRequest_.put("token", token_);

	ptree ptPhrases, ptPhrase;
	for(Phrases::const_iterator it = phrases_.begin(); it != phrases_.end(); ++it)
	{
	    ptPhrase.put("", *it);
	    ptPhrases.push_back(std::make_pair("", ptPhrase));
	}

	ptree ptGeoIds, ptGeoId;
	ptGeoId.put("", 0);
	ptGeoIds.push_back(std::make_pair("", ptGeoId));

	ptree ptParam;
	ptParam.add_child("Phrases", ptPhrases);
	ptParam.add_child("GeoID", ptGeoIds);

	ptRequest_.add_child("param", ptParam);
    }

    void YdrCreateWsReport::run()
    {
	YdRequest::run();
    }

    void YdrCreateWsReport::processResult()
    {
	YdRequest::processResult();

	YdRemote::ReportIdType reportId;
	if(getNodeVal<YdRemote::ReportIdType>(ptResponse_, "data", reportId))
	{
	    reportId_ = reportId;
	    state_ = ydOk;
	}
	else
	{
	    state_ = ydDataParseError;
	}

	runYdProcessCallback();
    }

    YdRemote::ReportIdType YdrCreateWsReport::getReportId()
    {
	return reportId_;
    }
}
