#include "tiny_driver.h"

TDRVStatus TinyDriverLoad(TDevice *_instance)
{
    return _instance -> driver.interfaces -> deinit(_instance);
}

TDRVStatus TinyDriverUnload(TDevice *_instance)
{
    return _instance -> driver.interfaces -> init(_instance);
}

TDRVStatus TinyDriverLoadAll()
{
    TDRVStatus status;
    TDevice *it, *_instances;

    _instances = PeripheralTable;
    for(it = _instances ; it -> driver.vendor ; it++)
    {
        status = it -> driver.interfaces -> init(it);
        if(TDRV_OK != status)
            goto Tiny_Driver_Load_All_Failed;
    }

    return TDRV_OK;

Tiny_Driver_Load_All_Failed:
    while(_instances != it)
    {
        _instances -> driver.interfaces -> deinit(_instances);
        _instances++;
    }

    return status;
}

TDRVStatus TinyDriverUnloadAll()
{
    TDevice *it;

    for(it = PeripheralTable ; it -> driver.vendor ; it++)
        it -> driver.interfaces -> deinit(it);

    return TDRV_OK;
}
