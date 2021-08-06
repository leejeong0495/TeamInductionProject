const express = require('express');
const app = express();

const SerialPort = require('serialport');
const sp = new SerialPort('COM5', {
  baudRate: 115200
});

const host = '192.168.0.9';
const port = 11540;

var cnt = 0;	// 온도 변수
var num = 0;	// 타이머 변수
var powerFlag = false;	// 전원 꺼져있을때
var lockFlag = false;	// 잠겨있을때
var timerFlag = false;	// 타이머에 드갈애
var weighFlag = false;

// 전원
app.get('/led_on',function(req,res)
{
  sp.write('0\n\r', function(err){
    if (err) {
      return console.log('Error on write: ', err.message);
    }
	
	if(lockFlag == true){
		powerFlag = false;
	}
	else {
		powerFlag = true;
		console.log('" 전원이 켜졌습니다. "\n');
		res.writeHead(200, {'Content-Type': 'text/plain'});
		res.end('LED On\n');
	}
  });
});

app.get('/led_off',function(req,res)
{
  sp.write('1\n\r', function(err){
    if (err) {
      return console.log('Error on write: ', err.message);
    }
	
	if(lockFlag == true){
		powerFlag = false;
	}
	else{
		powerFlag = false;
		cnt = 0;
		console.log('" 전원이 꺼졌습니다. "\n');
		res.writeHead(200, {'Content-Type': 'text/plain'});
		res.end('LED Off\n');
		
	}
  });
});

// 잠금
app.get('/lock_on',function(req,res)
{
  sp.write('2\n\r', function(err){
    if (err) {
      return console.log('Error on write: ', err.message);
    }
	
	if(powerFlag == true)
	{
		lockFlag = false;
	}
	else{
		lockFlag = true;
		console.log('" 인덕션을 잠급니다. "\n');
		res.writeHead(200, {'Content-Type': 'text/plain'});
		res.end('LOCK On\n');
	}	
	
  });
});

app.get('/lock_off',function(req,res)
{
  sp.write('3\n\r', function(err){
    if (err) {
      return console.log('Error on write: ', err.message);
    }
	
	if(powerFlag == true){
		lockFlag = false;
	}
	else{
		lockFlag = false;
		console.log('" 잠금이 해제되었습니다. "\n');
		res.writeHead(200, {'Content-Type': 'text/plain'});
		res.end('LOCK Off\n');
	}
	
  });
});

// 온도
app.get('/tem_up',function(req,res)
{
  sp.write('4\n\r', function(err){
    if (err) {
      return console.log('Error on write: ', err.message);
    }
	
	if(powerFlag == true && weighFlag == true){
		console.log('" 온도 up "\n');
		res.writeHead(200, {'Content-Type': 'text/plain'});

		cnt += 1;
	
		if(cnt > 8)
		{
			cnt = 8;
		}
		console.log('온도가 증가했습니다. %d단 \n', cnt);
		res.end(cnt + '단 tem up');
	}
	else{
		cnt = 0;
	}
  });
});

app.get('/tem_down',function(req,res)
{
  sp.write('5\n\r', function(err){
    if (err) {
      return console.log('Error on write: ', err.message);
    }
	
	if(powerFlag == true && weighFlag == true){
		console.log('"온도 down"\n');
		res.writeHead(200, {'Content-Type': 'text/plain'});
	
		cnt -= 1;

		if(cnt < 0){
			cnt=0;
			console.log('" 꺼졌습니다. "\n');
		}
		
		console.log('온도가 감소했습니다. %d단 \n',cnt);
		res.end(cnt + '단 Tem Down\n');
	}
	
	else{
		cnt = 0;
	}
  });
});

// 타이머
app.get('/time_up',function(req,res)
{
  sp.write('6\n\r', function(err){
    if (err) {
      return console.log('Error on write: ', err.message);
    }
	
	if(powerFlag == true &&  weighFlag == true){
		console.log('" 시간 up! "\n');
		res.writeHead(200, {'Content-Type': 'text/plain'});
			
		var interval = null;
	
		num += 30;
		t = num;
		if(timerFlag == false){
			timerFlag = true;
				
			interval = setInterval(() => {
				if(powerFlag == true){
					t = t - 1;	// 1씩 카운트 다운
					if(num < 0 || t < 0){
						t = 0;
						num = 0;
						console.log(' 끝났습니다. ');
						console.log("사용했던 시간은 %d 입니다. " , num);
						clearInterval(interval);
						timerFlag = false;
					}
					console.log('" %d 남았습니다. "', t);
				}
				else if(powerFlag == false){
					timerFlag = false;
					num = 0;
					t = 0;
					clearInterval(interval);
				}
			}, 1000);	// 1초씩마다
		}
		console.log('" 시간이 올라가요 + %d"\n', num);
		res.end('시간: '+num+' up\n');
	}
  });
});

app.get('/time_down',function(req,res)
{
  sp.write('7\n\r', function(err){
    if (err) {
      return console.log('Error on write: ', err.message);
    }
	
	if(powerFlag == true && weighFlag == true){
		console.log('" 시간 down! "\n');
		res.writeHead(200, {'Content-Type': 'text/plain'});

		num -= 30;
		t = num;
		if(num <= 0 || t < 0){
			num = 0;
			t = 0;
			timerFlag = false;
			console.log('" 끝! "\n');
		}
		console.log('" 시간 내려가요 %d"\n', t);
		res.end('시간: '+num+' down\n');
	}
  });
});

app.get('/dma_up',function(req,res)
{
  sp.write('8\n\r', function(err){
    if (err) {
      return console.log('Error on write: ', err.message);
    }
	
	weighFlag = true;
	
	if(powerFlag == true){
		console.log('" 물건이 올라가 있습니다. "\n');
		res.writeHead(200, {'Content-Type': 'text/plain'});
		res.end('Up\n');
	}
  });
});

app.get('/dma_down',function(req,res)
{
  sp.write('9\n\r', function(err){
    if (err) {
      return console.log('Error on write: ', err.message);
    }
	
	weighFlag = false;
	
	if(powerFlag == true){
		console.log('" 물건이 올라가 있지 않습니다. "\n');
		res.writeHead(200, {'Content-Type': 'text/plain'});
		res.end('Down\n');
	}
  });
});

app.use(express.static(__dirname + '/public'));

app.listen(port, host, function(){
  console.log('listening on *:' + port);
});
