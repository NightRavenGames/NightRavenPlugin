#pragma once

#include "Modules/ModuleManager.h"

class FCustomBlockoutsModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    void RegisterCategory();
    void RegisterBlockoutMesh(
        const FName& CategoryHandle,
        const TCHAR* AssetPath,
        const FText& DisplayName,
        int32 SortOrder,
        FName IconName = NAME_None
        );
};
