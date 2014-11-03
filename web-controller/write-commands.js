var i2c = require('i2c');
var device1 = new i2c(0x18, {device: '/dev/i2c-1', debug: false});
device1.setAddress(0x4);

var horizontalMotion = 127;
var verticalMotion = 127;
var fire = 0;
var a = 1;
var b = 1;
var y = 1;
var x = 1;


var sendBuffer = [horizontalMotion,verticalMotion,fire,a,b,y,x];


while(1)
{

	if(1<2)
	{
		up();
	}
requestData();

}




function up()
{
	for (i=0;i<sendBuffer.length;i++)
	{
		//console.log(sendBuffer[i]);
		device1.writeByte(sendBuffer[i],function(err){});
		sendBuffer[i]++;
	}
}
//device1.writeByte(115,function(err){});

//console.log(device1.readByte(function(err, res) {return res; }));


function requestData()
{
	console.log("Reading: " + device1.readByte(function(err,res) {return res;}));
	console.log("Reading: " + device1.readByte(function(err,res) {return res;}));
}
