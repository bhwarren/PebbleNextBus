Pebble.addEventListener("ready",
	function(e) {
		console.log("JavaScript app ready and running!");
		var req = new XMLHttpRequest();
		req.open('GET', 'http://api.openweathermap.org/data/2.1/find/city?lat=37.830310&lon=-122.270831&cnt=1', true);
		req.onload = function(e) {
			if (req.readyState == 4 && req.status == 200) {
				if(req.status == 200) {
					var response = JSON.parse(req.responseText);
					var temperature = response.list[0].main.temp;
					var icon = response.list[0].main.icon;
					Pebble.sendAppMessage({ "icon":icon, "temperature":temperature + "\u00B0C"});
				} else { 
					console.log("Error"); 
				}
			}
		};
		req.send(null);
	}
);


