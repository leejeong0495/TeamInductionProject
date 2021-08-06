<!doctype html>
<html lang="ko">
  <head>
    <meta charset="utf-8">
    <title>사용 시간</title>
    <style>
      body {
        font-family: Consolas, monospace;
        font-family: 12px;
      }
      table {
        width: 100%;
      }
      th, td {
        padding: 10px;
        border-bottom: 1px solid #dadada;
      }
    </style>
  </head>
  <body>    
    <table>
      <thead>
        <tr>
          <th>No.</th>
          <th>Time(minute)</th>
        </tr>
      </thead>
      <tbody>
        <?php
          $connect = mysqli_connect( 'localhost', 'root', 'apmsetup', 'induction' );
          $sql = "SELECT * FROM induction_log;";
          $result = mysqli_query( $connect, $sql );
          while( $row = mysqli_fetch_array( $result ) ) {
            echo '<tr><td><center>' . $row[ 'NO' ] . '</center></td><td><center>'. $row[ 'time' ] . '</center></td><td><center></td></tr>';

          }
        ?>
      </tbody>
    </table>
  </body>
</html>

<!-- 뒤로가기 버튼 -->
<script type = "text/javascript">
    function goBack(){
        window.history.back();
    }
</script>
<input type="button" value = "뒤로가기" onclick="goBack();"/>
<!DOCTYPE html>