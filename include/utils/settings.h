#pragma once
#include <QString>
#include <QJsonDocument>

///
/// @brief Provide util methods to work with SettingsManager class
///
namespace settings {
// all available settings sections
enum type  {
	BGEFFECT,
	FGEFFECT,
	BLACKBORDER,
	BOBLSERVER,
	COLOR,
	DEVICE,
	EFFECTS,
	NETFORWARD,
	SYSTEMCAPTURE,
	GENERAL,
	V4L2,
	JSONSERVER,
	LEDCONFIG,
	LEDS,
	LOGGER,
	PROTOSERVER,
	SMOOTHING,
	UDPLISTENER,
	WEBSERVER,
	INSTCAPTURE,
	NETWORK,
	INVALID
};

///
/// @brief Convert settings::type to string representation
/// @param  type  The settings::type from enum
/// @return       The settings type as string
///
inline QString typeToString(const type& type)
{
	switch (type)
	{
		case BGEFFECT:      return "backgroundEffect";
		case FGEFFECT:      return "foregroundEffect";
		case BLACKBORDER:   return "blackborderdetector";
		case BOBLSERVER:    return "boblightServer";
		case COLOR:         return "color";
		case DEVICE:        return "device";
		case EFFECTS:       return "effects";
		case NETFORWARD:    return "forwarder";
		case SYSTEMCAPTURE: return "framegrabber";
		case GENERAL:       return "general";
		case V4L2:          return "grabberV4L2";
		case JSONSERVER:    return "jsonServer";
		case LEDCONFIG:     return "ledConfig";
		case LEDS:          return "leds";
		case LOGGER:        return "logger";
		case PROTOSERVER:   return "protoServer";
		case SMOOTHING:     return "smoothing";
		case UDPLISTENER:   return "udpListener";
		case WEBSERVER:     return "webConfig";
		case INSTCAPTURE:   return "instCapture";
		case NETWORK:       return "network";
		default:            return "invalid";
	}
}

///
/// @brief Convert string to settings::type representation
/// @param  type  The string to convert
/// @return       The settings type from enum
///
inline type stringToType(const QString& type)
{
	if (type == "backgroundEffect")     return BGEFFECT;
	if (type == "foregroundEffect")     return FGEFFECT;
	if (type == "blackborderdetector")  return BLACKBORDER;
	if (type == "boblightServer")       return BOBLSERVER;
	if (type == "color")                return COLOR;
	if (type == "device")               return DEVICE;
	if (type == "effects")              return EFFECTS;
	if (type == "forwarder")            return NETFORWARD;
	if (type == "framegrabber")         return SYSTEMCAPTURE;
	if (type == "general")              return GENERAL;
	if (type == "grabberV4L2")          return V4L2;
	if (type == "jsonServer")           return JSONSERVER;
	if (type == "ledConfig")            return LEDCONFIG;
	if (type == "leds")                 return LEDS;
	if (type == "logger")               return LOGGER;
	if (type == "protoServer")          return PROTOSERVER;
	if (type == "smoothing")            return SMOOTHING;
	if (type == "udpListener")          return UDPLISTENER;
	if (type == "webConfig")            return WEBSERVER;
	if (type == "instCapture")          return INSTCAPTURE;
	if (type == "network")              return NETWORK;
	                                    return INVALID;
}
};