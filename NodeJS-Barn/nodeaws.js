var SerialPort = require('serialport')
const request = require('request')
var serialPort = new SerialPort("COM7", {
  baudRate: 9600
});

serialPort.on('open', function() {
	console.log('shits open')
});

var hold = '';

var _timeout = null;

function sendDataToAws(id, lat, lng) {
	
	if (!_timeout) {
		
		_timeout = setTimeout(function() {
			
		var options = {
		  uri: 'https://chhklypx0b.execute-api.us-east-1.amazonaws.com/pro/cows/' + id,
		  method: 'POST',
		  json: {
			'lat': lat,
			'lng': lng
		  }
		};

		request(options, function (err, res, data) {
		  if (err) { return console.log(err); }
		  try {
			console.log(data.message);
		  } catch (e) {
			console.log('aww shiet', e)
		  }
		});			
			
			
			_timeout = null;
		}, 5000);
	}
}

serialPort.on("data", function(data) {
	var s = data.toString('utf-8')
		
	hold += s;
	
	if (hold[hold.length - 1] === ';') {
		parts = hold.slice(0,-1).split(',');
		
		id = parts[0];
		lat = parts[1];
		lng = parts[2];
	
		sendDataToAws(id, lat, lng)
	
		// clear hold
		hold = '';
	}
});

/*
while(1){
var text = serialPort.read();
console.log(text);
}*/