<form name="Filter" method="POST">
    <select name="sensor" >
      <option value="Water, Temp, Hum, Ilu" selected>Water, Temp, Hum, Ilu</option>
      <option value="Water">Water</option>
      <option value="Temp">Temp</option>
      <option value="Humi">Hum</option>
      <option value="Ilu">Ilu</option>
    </select>
    <input type="date" name="dateFrom" value="<?php echo date('Y-m-d'); ?>" min="2021-08-07" />
    <input type="date" name="dateFrom2" value="<?php echo date('Y-m-d'); ?>" />
    <input type="submit" name="submit" value="Search"/>
</form>
<?php
	$sensor_name = $_POST['sensor'];
	$new_date = date('Y-m-d', strtotime($_POST['dateFrom']));
	$new_date1 = date('Y-m-d', strtotime($_POST['dateFrom2']));
	
        if($sensor_name == '')
                $sensor_name = 'Water, Temp, Hum, Ilu';

	$new_date = date('Y-m-d');
	$new_date1 = $new_date;
	$printDate = 0;
	$query = "SELECT DATE(date) as date, round(AVG(Water),0) as Water, round(AVG(Temp),0) as Temp,round(AVG(Hum),0) as Hum, round(AVG(Ilu),0) as Ilu FROM SENSOR GROUP BY DATE(date)";
	
	$conn = mysqli_connect("localhost", "pi", "qwer1234");
#	mysqli_set_charset($conn, "UTF-8");
	mysqli_select_db($conn, "mini_project");
	$result = mysqli_query($conn, $query);

	$data = array(array('ARD','Water','Temp','Hum','Ilu'));

	if($result)
	{
		while($row = mysqli_fetch_array($result))
		{
			if($printDate == 0)
				array_push($data, array($row['time'], intval($row['Water']),floatval($row['Temp']),floatval($row['Hum']) ,floatval($row['Ilu'])));
			else	
				array_push($data, array($row['date']." ".$row['time'], intval($row['Water']), floatval($row['Temp']), floatval($row['Hum']) ,floatval($row['Ilu'])));
		}
	}

	$options = array(
			'title' => 'Revolution of Rooftop',
			'width' => 1000, 'height' => 400,
			'curveType' => 'function'
			);

?>

<script src="//www.google.com/jsapi"></script>
<script>
var data = <?=json_encode($data) ?>;
var options = <?= json_encode($options) ?>;

google.load('visualization', '1.0', {'packages':['corechart']});

google.setOnLoadCallback(function() {
	var chart = new google.visualization.LineChart(document.querySelector('#chart_div'));
	chart.draw(google.visualization.arrayToDataTable(data), options);
	});
	</script>
<div id="chart_div"></div>
