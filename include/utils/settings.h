#pragma once
#include <QString>

namespace settings {
// all available settings sections
enum type  {
	S_BGEFFECT,
	S_FGEFFECT,
	S_BLACKB,
	S_BOBLSERVER,
	S_COLOR,
	S_DEVICE,
	S_EFFECTS,
	S_FORWARD,
	S_PCAPTURE,
	S_GENERAL,
	S_V4L2,
	S_JSONSERVER,
	S_LEDCONFIG,
	S_LEDS,
	S_LOGGER,
	S_PROTOSERVER,
	S_SMOOTHING,
	S_UDPLISTENER,
	S_WEBSERVER,
	S_INVALID
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
		case S_BGEFFECT:    return "backgroundEffect";
		case S_FGEFFECT:    return "foregroundEffect";
		case S_BLACKB:      return "blackborderdetector";
		case S_BOBLSERVER:  return "boblightServer";
		case S_COLOR:       return "color";
		case S_DEVICE:      return "device";
		case S_EFFECTS:     return "effects";
		case S_FORWARD:     return "forwarder";
		case S_PCAPTURE:    return "framegrabber";
		case S_GENERAL:     return "general";
		case S_V4L2:        return "grabberV4L2";
		case S_JSONSERVER:  return "jsonServer";
		case S_LEDCONFIG:   return "ledConfig";
		case S_LEDS:        return "leds";
		case S_LOGGER:      return "logger";
		case S_PROTOSERVER: return "protoServer";
		case S_SMOOTHING:   return "smoothing";
		case S_UDPLISTENER: return "udpListener";
		case S_WEBSERVER:   return "webConfig";
		default: return "invalid";
	}
}

///
/// @brief Convert string to settings::type representation
/// @param  type  The string to convert
/// @return       The settings type from enum
///
inline type stringToType(const QString& type)
{
	if      (type == "backgroundEffect")     return S_BGEFFECT;
	else if (type == "foregroundEffect")     return S_FGEFFECT;
	else if (type == "blackborderdetector")  return S_BLACKB;
	else if (type == "boblightServer")       return S_BOBLSERVER;
	else if (type == "color")                return S_COLOR;
	else if (type == "device")               return S_DEVICE;
	else if (type == "effects")              return S_EFFECTS;
	else if (type == "forwarder")            return S_FORWARD;
	else if (type == "framegrabber")         return S_PCAPTURE;
	else if (type == "general")              return S_GENERAL;
	else if (type == "grabberV4L2")          return S_V4L2;
	else if (type == "jsonServer")           return S_JSONSERVER;
	else if (type == "ledConfig")            return S_LEDCONFIG;
	else if (type == "leds")                 return S_LEDS;
	else if (type == "logger")               return S_LOGGER;
	else if (type == "protoServer")          return S_PROTOSERVER;
	else if (type == "smoothing")            return S_SMOOTHING;
	else if (type == "udpListener")          return S_UDPLISTENER;
	else if (type == "webConfig")            return S_WEBSERVER;
	else                                     return S_INVALID;
}
};
