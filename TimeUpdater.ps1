$MyValsLocation = "C:\Users\ryanr\Documents\GitHub\UV-Watch\SunSensor_Firmware\V4-SunSensor_Firmware\myVals.h"
$TimeOffSet = New-TimeSpan -Days 0 -Hours 0 -Minutes 0 -Seconds 15

while($TRUE){
    $CurrentTime = (Get-Date) + $TimeOffSet
    $PSHour = "int ExtClockHour = $($CurrentTime.ToString("hh")  -replace '^0+', '0');"
    $PSMinute = "int ExtClockMinute = $($CurrentTime.ToString("mm") -replace '^0+', '0' );"
    $PSSecond = "int ExtClockSecond = $($CurrentTime.ToString("ss") -replace '^0+', '0' );"

    $PSHour | out-file $MyValsLocation -Encoding ASCII
    $PSMinute | Add-Content $MyValsLocation -Encoding ASCII
    $PSSecond | Add-Content $MyValsLocation -Encoding ASCII
    
    write-host $PSHour
    write-host $PSMinute
    write-host $PSSecond
    write-host "---------------"

    Start-Sleep -Seconds 10
}