// global array holding messages to be sent
var bus_msg_que = [];

var grabbed_data = false; // false until web scraping finishes


//keep position in global vars
var lat;
var lon;

var locationOptions = {
  enableHighAccuracy: true, 
  maximumAge: 10000, 
  timeout: 10000
};

//set location on success
function locationSuccess(pos) {
	lat = pos.coords.latitude;
	lon = pos.coords.longitude;
}

//restore defaults if can't get gps
function locationError(err) {
  lat = "35.910092";
	lon = "-79.05321789999999";
}


// Function to send a message to the Pebble using AppMessage API
function sendMessage() {
	
	var req = new XMLHttpRequest();
	
	console.log("lat:"+lat.toString()+"  lon:"+lon.toString());
	
	var url = "https://www.nextbus.com/webkit/predsByLoc.jsp?lat="+lat+"&"+"lon="+lon+
							"&maxDis=2300&maxNumStops=99&accuracy=30&"+
							"caller=showPredsBasedOnLoc&moreStopsSelected=chapel-hill";
							
	//var url2 = "https://www.nextbus.com/webkit/predsByLoc.jsp?lat=35.910092&+lon=-79.05321789999999&maxDis=2300&maxNumStops=99&accuracy=30&caller=showPredsBasedOnLoc&moreStopsSelected=chapel-hill";						
	
	req.open("GET", url, true );
	req.responseType = "document";
	//req.onload = 
	req.onload =	function(e) {
		if (req.readyState == 4 && req.status == 200) {
			var currentNode = req.responseXML.getElementsByClassName("plainText")[0].childNodes[0].nextSibling;
			
			var route;
			var direction;
			var stop;
			var estimate;
			var num_stops_recorded = 0;
			
			var all_messages = "|";

			while (currentNode && num_stops_recorded < 20) {
				
				if (currentNode.className == "inBetweenRouteSpacer") {
					//route = currentNode.childNodes[1].innerHTML;
					route = currentNode.getElementsByClassName("routeName")[0].innerHTML;
				} 
				else if (currentNode.className == "link normalDirLinkColor" ||
							currentNode.className == "link altDirLinkColor") {
					
					
					/*direction = currentNode.childNodes[1].childNodes[1].innerHTML;
					stop = currentNode.childNodes[1].childNodes[3].data.trim().substr(6);
					estimate = currentNode.childNodes[1].childNodes[7].innerHTML.split(" ")[0];*/
					
						direction = currentNode.getElementsByClassName("directionName")[0].innerHTML; 
						stop = currentNode.getElementsByClassName("predsForStop")[0].childNodes[3].data.trim().substr(6);
						estimate = currentNode.getElementsByClassName("preds")[0].innerHTML.split(" ")[0];

					if (estimate != "Arriving") {
						estimate = estimate + " min";
					}
					
					num_stops_recorded += 1;
					
					var text = route + "\n" +
											direction + "\n" +
											stop + "\n" + 
											estimate + "\n";
					
					console.log(text);
					all_messages = all_messages+text+"|";
					
					
					// place data to be send on a queue recorded data to pebble
					//bus_msg_que.push(msg);
					
				}
				else if (currentNode.id.slice(0,15) != "moreStopsButton" && currentNode.id.slice(0,9) == "moreStops"){
					//console.log("hit it:"+currentNode.nextElementSibling.id);
					console.log("hit it: "+currentNode.childNodes[1].textContent);
					currentNode = currentNode.childNodes[0]; //skips over 0th because it's always undefined
				}
				//console.log("slices: "+currentNode.id.slice(0,14) +" "+currentNode.id.slice(0,8));
				//currentNode = currentNode.nextElementSibling;
				currentNode = currentNode.nextElementSibling;
			}
			
			grabbed_data = true;
			
						//console.log("BEFORE SUBSTRINGING IT:"+all_messages);

			// place data to be send on a queue recorded data to pebble
			all_messages=all_messages.substr(0,all_messages.length);
			//console.log("SENDING THIS TO PEBBLE:"+all_messages);
			bus_msg_que.push({"1": all_messages});
			
			//tell the c part that we're done
			//bus_msg_que.push({"1":"done"});
			
		} else {
			Pebble.sendAppMessage({"2": "Error connecting to server."});
		}
		
		sendMsg();
		
		
	};
	req.send();
	
	// PRO TIP: If you are sending more than one message, or a complex set of messages, 
	// it is important that you setup an ackHandler and a nackHandler and call 
	// Pebble.sendAppMessage({ /* Message here */ }, ackHandler, nackHandler), which 
	// will designate the ackHandler and nackHandler that will be called upon the Pebble 
	// ack-ing or nack-ing the message you just sent. The specified nackHandler will 
	// also be called if your message send attempt times out.
}

function sendMsg() {
	if (grabbed_data === false) {
		console.log("grabbed data is false, trying again");
		sendMessage();
	}
	else if (bus_msg_que.length > 0) {
		Pebble.sendAppMessage(bus_msg_que[0], sendNextMsg, sendMsg);
	}
}

function sendNextMsg() {
	if (bus_msg_que.length > 0) {
		bus_msg_que.shift();
		sendMsg();
	}
}


// Called when JS is ready
/*Pebble.addEventListener("ready", function(e) {
	sendMessage();
});	*/										

// Called when incoming message from the Pebble is received
Pebble.addEventListener("appmessage",	
								function(e) {
									console.log(e.payload.status);
									console.log(e.payload.message);

									if(e.payload.message == "hif"){
										console.log("getting buses !!!!!!!!!!!!!!!!!!");
										sendMessage();
									}
								}
							);

//find gps on startup
Pebble.addEventListener('ready',
  function(e) {
    // Request current position
    navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
  }
);
