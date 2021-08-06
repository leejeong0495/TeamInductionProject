<?php 
header("Content-Type: text/html;charset=UTF-8"); 

$host = 'localhost'; 
$user = 'root'; 
$pw = 'apmsetup'; 
$dbName = 'induction'; 
$mysqli = new mysqli($host, $user, $pw, $dbName); 

	if($mysqli){ 
		echo "MySQL successfully connected!<br/>"; 
		
		$NO = $_GET['NO']; 
		$time = $_GET['time']; 
		
		echo "<br/>number = $NO"; 
		echo ", "; 
		echo "using time = $time<br/>"; 
		
		$query = "INSERT INTO induction_log (NO, time) VALUES ('$NO','$time')"; 
		mysqli_query($mysqli,$query); 
		
		echo "</br>success!!"; 
	} 
	else{ 
		echo "MySQL could not be connected"; 
	} 
mysqli_close($mysqli); 
?>
