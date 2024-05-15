if ($(Get-Service ssh-agent | Select-Object -ExpandProperty StartType) -eq "Disabled") {
    Write-Output "[$PSScriptRoot] Service [ssh-agent] is [Disabled], setting it to [Manual]"
    Get-Service -Name ssh-agent | Set-Service -StartupType Manual
}
if ((Get-Service -Name ssh-agent).Status -ne "Running"){
    Write-Output "[$PSScriptRoot] Starting service [ssh-agent]"
    Start-Service ssh-agent
}
else {
    Write-Output "[$PSScriptRoot] Service [ssh-agent] is always running"
}