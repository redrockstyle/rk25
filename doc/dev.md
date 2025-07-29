# Commands

## Windows

### enable debug
```
bcdedit /debug on
bcdedit /dbgsettings serial debugport:1 baudrate:115200
```

### unsigned driver
`bcdedit -set testsigning on`

### pipe COM port
`\\.\pipe\WinName`

## WinDBG

### enable debug print
`ed nt!Kd_Default_Mask 0xf`

### mapping file from host
`.kdfiles -m \??\C:\Path\To\VM C:\Path\To\Host`

### ssdt address
`dps nt!KeServiceDescriptorTable L4`

### get syscall (example)
```
0: kd> dps nt!KeServiceDescriptorTable L4
fffff802`050098c0  fffff802`042d5100 nt!KiServiceTable
fffff802`050098c8  00000000`00000000
fffff802`050098d0  00000000`000001e6
fffff802`050098d8  fffff802`042d589c nt!KiArgumentTable
0: kd> dd /c1 nt!KiServiceTable+(4*0x36) L1
fffff802`042d51d8  05d25c00
0: kd> u nt!KiServiceTable + (05d25c00 >>> 4)
nt!NtQuerySystemInformation:
fffff802`048a76c0 4053            push    rbx
fffff802`048a76c2 4883ec30        sub     rsp,30h
fffff802`048a76c6 4533d2          xor     r10d,r10d
fffff802`048a76c9 458bd8          mov     r11d,r8d
fffff802`048a76cc 664489542440    mov     word ptr [rsp+40h],r10w
fffff802`048a76d2 488bda          mov     rbx,rdx
fffff802`048a76d5 81f9b5000000    cmp     ecx,0B5h
fffff802`048a76db 7d3c            jge     nt!NtQuerySystemInformation+0x59 (fffff802`048a7719)
```

### get module address (example)
```
0: kd> lm m nt
Browse full module list
start             end                 module name
fffff802`04208000 fffff802`0524f000   nt         (pdb symbols)
0: kd> lm m driver
Browse full module list
start             end                 module name
fffff802`2d840000 fffff802`2d84b000   driver     (deferred)
```

### info module
`!dh nt -f`

### info section
`!dh <base_address_module>`

### logfile
```
.logopen C:\path\to\local\log.txt
.logclose
```

---

## Visual Studio

### building old version
in file WindowsDriver.Common.targets:
 ```
    <!-- Error out if Arch x86 or ARM is used with KM drivers -->
    <Error Text=" '$(Platform)' is not a valid architecture for Kernel mode drivers or UMDF drivers"
           Condition="'$(WindowsTargetPlatformVersion)' &gt; '$(TargetPlatformVersion_CO)' and ('$(DDKPlatform)' == 'x86' or '$(DDKPlatform)' == 'ARM') and ('$(PlatformToolset)' == 'WindowsKernelModeDriver10.0' or '$(DriverType)' == 'UMDF') " />
    <!-- Error out if drivers are targeting OS Version lower than 10 -->
    <Error Text=" '$(TargetVersion)' is not a supported OS Version"
           Condition="'$(WindowsTargetPlatformVersion)' &gt; '$(TargetPlatformVersion_CO)' and '$(TargetVersion)' !='$(LatestTargetVersion)' " />
```
chandge to (comment):
```
    <!-- Error out if Arch x86 or ARM is used with KM drivers -->
	<!--<Error Text=" '$(Platform)' is not a valid architecture for Kernel mode drivers or UMDF drivers"
           Condition="'$(WindowsTargetPlatformVersion)' &gt; '$(TargetPlatformVersion_CO)' and ('$(DDKPlatform)' == 'x86' or '$(DDKPlatform)' == 'ARM') and ('$(PlatformToolset)' == 'WindowsKernelModeDriver10.0' or '$(DriverType)' == 'UMDF') " />-->
    <!-- Error out if drivers are targeting OS Version lower than 10 -->
	<!--<Error Text=" '$(TargetVersion)' is not a supported OS Version"
           Condition="'$(WindowsTargetPlatformVersion)' &gt; '$(TargetPlatformVersion_CO)' and '$(TargetVersion)' !='$(LatestTargetVersion)' " />-->
```