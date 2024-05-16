param ($use_recommended) # -use_recommended yes

$ssh_agent_startType=$(Get-Service ssh-agent | Select-Object -ExpandProperty StartType)
# Automatic, AutomaticDelayedStart, Disabled, InvalidValue, Manual
if (
    ('Disabled','InvalidValue' -contains ${ssh_agent_startType}) -or
    ('yes','y' -contains ${use_recommended})
) {
    Write-Output "[$PSScriptRoot] Service [ssh-agent] is [${ssh_agent_startType}], setting it to [AutomaticDelayedStart] ..."
    $currentPrincipal = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
    if (-Not $currentPrincipal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)){
        Write-Error "[$PSScriptRoot] Requires Administrator privilege to proceed."
    }
    Get-Service -Name ssh-agent | Set-Service -StartupType Automatic
}
elseif (${ssh_agent_startType} -eq "Manual"){
    Write-Warning "[$PSScriptRoot] Service [ssh-agent] is [Manual], setting it to [AutomaticDelayedStart ] is recommended"
}

if ((Get-Service -Name ssh-agent).Status -ne "Running"){
    Write-Output "[$PSScriptRoot] Starting service [ssh-agent]"
    Start-Service ssh-agent
}
else {
    Write-Output "[$PSScriptRoot] Service [ssh-agent] is always running"
}
