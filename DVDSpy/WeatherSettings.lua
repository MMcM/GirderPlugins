-- $Header$

-- Location(s) to get weather data for (can use ZIP codes for USA).
-- (Some tweaking of the GML samples will be required if more than one
-- entry is added, to keep the displays straight.)
AddWeatherLocation("NLXX0002")  -- Amsterdam

-- Interval (in ms.) between downloads of weather data.
WeatherReloadInterval = 600000

-- Interval (in ms.) after which old data is considered invalid.
WeatherInvalidInterval = 3600000
