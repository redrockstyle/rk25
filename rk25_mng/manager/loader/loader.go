package loader

import (
	"log"
	"rk25_mng/manager/loader/winhelper"
)

type DriverLoader struct {
	service string
	path    string
}

func NewDriverLoader(service string, path string) *DriverLoader {
	return &DriverLoader{service: service, path: path}
}

func (dl *DriverLoader) RegisterService() error {
	log.Printf("Register service: %v\n", dl.service)
	scHandle, err := winhelper.OpenSCManager()
	if err != nil {
		log.Printf("Error OpenSCManager: %v\n", err)
		return err
	}
	defer winhelper.CloseServiceHandle(scHandle)

	svcHandle, err := winhelper.CreateService(scHandle, dl.service, dl.service, dl.path)
	if err != nil {
		log.Printf("Error CreateService: %v\n", err)
		return err
	}
	defer winhelper.CloseServiceHandle(svcHandle)

	log.Printf("Service %v registered successfully\n", dl.service)
	return err
}

func (dl *DriverLoader) UnregisterService() error {
	log.Printf("Remove service: %v\n", dl.service)
	svcHandle, err := winhelper.GetServiceHandle(dl.service)
	if err != nil {
		log.Printf("Error GetServiceHandle: %v\n", err)
		return err
	}
	defer winhelper.CloseServiceHandle(svcHandle)

	if err := winhelper.DeleteService(svcHandle); err != nil {
		log.Printf("DeleteService failed: %v\n", err)
	}

	log.Printf("Service %v removed successfully\n", dl.service)
	return nil
}

func (dl *DriverLoader) StartService() error {
	log.Printf("Start driver: %v\n", dl.path)
	svcHandle, err := winhelper.GetServiceHandle(dl.service)
	if err != nil {
		log.Printf("Error GetServiceHandle: %v\n", err)
		return err
	}
	defer winhelper.CloseServiceHandle(svcHandle)

	if err := winhelper.StartService(svcHandle); err != nil {
		log.Printf("Error StartService: %v\n", err)
		return err
	}
	log.Println("Driver started successfully")
	return nil
}

func (dl *DriverLoader) StopService() error {
	log.Printf("Stop driver: %v\n", dl.path)
	svcHandle, err := winhelper.GetServiceHandle(dl.service)
	if err != nil {
		log.Printf("Error GetServiceHandle: %v\n", err)
		return err
	}
	defer winhelper.CloseServiceHandle(svcHandle)

	if err := winhelper.StopService(svcHandle); err != nil {
		log.Printf("StopService failed: %v\n", err)
	}

	log.Println("Driver stoped successfully")
	return nil
}

func (dl *DriverLoader) LoadDriver() error {
	if err := dl.RegisterService(); err != nil {
		return err
	}

	if err := dl.StartService(); err != nil {
		return err
	}

	log.Println("Driver installed successfully")
	return nil
}

func (dl *DriverLoader) UnloadDriver() error {
	if err := dl.UnregisterService(); err != nil {
		return err
	}

	if err := dl.StopService(); err != nil {
		return err
	}

	log.Println("Driver removed successfully")
	return nil
}
