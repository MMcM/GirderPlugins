// Get the weather.
// $Header$

// Arguments: accid...
// ACCID is the location (ZIP code in the USA).
// Cf. http://www.msnbc.com/news/wea_front.asp?ta=y&tab=BW&tp=&ctry=&cp1=1
var args = WScript.Arguments;

// This progid seems to be the generally available one.
// WinHttp.WinHttpRequest.5 also works, if that's all you have; nothing
// depends on any differences.
var HttpReq = WScript.CreateObject("WinHttp.WinHttpRequest.5.1");

// This can be converted to the Girder event object as soon as that is
// fully supported.
var GirderEvent = WScript.CreateObject("Girder.GirderEvent");
GirderEvent.Device = 215;
GirderEvent.EventString = "NewWeather";

function addVar(pld, name, val) {
  if (pld == null)
    pld = "";
  else
    pld = pld + ",";
  if (val == "")
    val = "nil";
  return pld + name + "=" + val;
}

function addVarStr(pld, name, val) {
  var strval = val.replace(/([\\\"\n])/g, "\\$1"); // Escape special characters.
  return addVar(pld, name, "\"" + strval + "\"");
}

for (var i = 0; i < args.Length; i++) {
  var accid = args(i);

  try {
    // Get the weather object; downloads as JavaScript source.
    HttpReq.Open("GET", "http://www.msnbc.com/m/chnk/d/weather_d_src.asp?acid=" + accid, false);
    HttpReq.Send();
    if (200 != HttpReq.Status()) continue;
    var src = HttpReq.ResponseText();
    eval(src);
    var weather = new makeWeatherObj();

    var payload = null;

    payload = addVarStr(payload, "City", weather.swCity);
    payload = addVarStr(payload, "State", weather.swSubDiv);
    payload = addVarStr(payload, "Country", weather.swCountry);
    payload = addVarStr(payload, "Region", weather.swRegion);
    payload = addVar(payload, "TempF", weather.swTemp);
    payload = addVar(payload, "TempC", weather.swTempCel);
    payload = addVar(payload, "WindSpeed", weather.swWindS);
    payload = addVarStr(payload, "WindDirection", weather.swWindD);
    payload = addVar(payload, "Barometer", weather.swBaro);
    payload = addVar(payload, "Humidity", weather.swHumid);
    payload = addVar(payload, "RealTemp", weather.swReal);
    payload = addVar(payload, "UVIndex", weather.swUV);
    payload = addVarStr(payload, "Visibility", weather.swVis);
    payload = addVarStr(payload, "LastUpdate", weather.swLastUp);
    payload = addVarStr(payload, "Conditions", weather.swConText);
    payload = addVarStr(payload, "Forecast", weather.swFore);
    payload = addVarStr(payload, "Accid", weather.swAcid);

    GirderEvent.Payload(1) = payload;
    GirderEvent.Send();
  }
  catch (ex) {
    Debug.writeln(ex);
  }
}
