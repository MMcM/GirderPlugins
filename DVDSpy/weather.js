// Get the weather.

// Arguments: accid xmlfile [prefix]
var args = WScript.Arguments

// ACCID is the location (ZIP code in the USA).
// Cf. http://www.msnbc.com/news/wea_front.asp?ta=y&tab=BW&tp=&ctry=&cp1=1
var accid = args(0);

// Write this file with infomation in Girder register format.
var xmlfile = args(1);

// Unique string if more than one city is fetched.
var prefix = "";
if (args.length > 2) prefix = args(2);

function addReg(elem, name, value) {
  name = "Weather" + prefix + name;
  var children, existing, child;
  children = elem.childNodes;
  existing = null;
  for (var i = 0; i < children.length; i++) {
    child = elem.childNodes(i);
    if (child.getAttribute("Name") == name) {
      existing = child;
      break;
    }
  }
  child = xml.createElement("Register");
  child.setAttribute("Name", name);
  var text = xml.createTextNode(value);
  child.appendChild(text);
  if (existing == null)
    elem.appendChild(child);
  else
    elem.replaceChild(child, existing);
}

try {

  // Get the weather object; downloads as JavaScript source.
  // This progid seems to be the generally available one.
  // WinHttp.WinHttpRequest.5 also works, if that's all you have; nothing
  // depends on any differences.
  var HttpReq = WScript.CreateObject("WinHttp.WinHttpRequest.5.1");
  HttpReq.Open("GET", "http://www.msnbc.com/m/chnk/d/weather_d_src.asp?acid=" + accid, false);
  HttpReq.Send();
  if (200 != HttpReq.Status()) exit;
  var src = HttpReq.ResponseText();
  eval(src);
  var weather = new makeWeatherObj();

  var xml = WScript.CreateObject("Msxml2.DOMDocument");
  xml.load(xmlfile);

// As of LUA conversion, there really aren't any integer registers.
//var ints = xml.getElementsByTagName("IntegerRegisters").item(0);
  var dbls = xml.getElementsByTagName("DoubleRegisters").item(0);
  var strs = xml.getElementsByTagName("StringRegisters").item(0);

  addReg(strs, "City", weather.swCity);
  addReg(strs, "State", weather.swSubDiv);
  addReg(strs, "Country", weather.swCountry);
  addReg(strs, "Region", weather.swRegion);
  addReg(dbls, "TempF", weather.swTemp);
  addReg(dbls, "TempC", weather.swTempCel);
  addReg(dbls, "WindSpeed", weather.swWindS);
  addReg(strs, "WindDirection", weather.swWindD);
  addReg(dbls, "Barometer", weather.swBaro);
  addReg(dbls, "Humidity", weather.swHumid);
  addReg(dbls, "RealTemp", weather.swReal);
  addReg(dbls, "UVIndex", weather.swUV);
  addReg(strs, "Visibility", weather.swVis);
  addReg(strs, "LastUpdate", weather.swLastUp);
  addReg(strs, "Conditions", weather.swConText);
  addReg(strs, "Forecast", weather.swFore);
  addReg(strs, "Accid", weather.swAcid);

  xml.save(xmlfile);

  // Tell Girder we're done.
  var shell = WScript.CreateObject("WScript.Shell");
  var cmd = "\"c:\\Program Files\\girder32\\Girder.exe\" -eventstring NewWeather"
  shell.Run(cmd, 0);

}
catch (ex) {
  Debug.writeln(ex);
}
