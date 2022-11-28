#pragma once

#include "EraseTasksCommandlet.h"

#include "Commandlets/Commandlet.h"
#if defined(UE_5_1_OR_LATER)
#include "AssetRegistry/AssetRegistryModule.h"
#else
#include "AssetRegistryModule.h"
#endif
#include "Developer/AssetTools/Public/AssetToolsModule.h"
#include "BlueprintGraph/Classes/K2Node_MacroInstance.h"
#include "BlueprintGraph/Classes/K2Node_SpawnActorFromClass.h"
#include "BlueprintGraph/Classes/K2Node_DynamicCast.h"
#include "Editor.h" 
#include "SourceControlHelpers.h"
#include "UObject/SavePackage.h"
#include "Misc/FeedbackContext.h"

int32 UEraseTasksCommandlet::Main(const FString& Params)
{
    auto &AssetRegistryModule =
            FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
    auto &AssetRegistry = AssetRegistryModule.Get();

    AssetRegistry.SearchAllAssets(true);
    while (AssetRegistry.IsLoadingAssets())
    {
        AssetRegistry.Tick(1.0f);
    }

    TArray<FAssetData> Assets;
    UClass *TargetClass = UBlueprint::StaticClass();
#if defined(UE_5_1_OR_LATER)
    AssetRegistry.GetAssetsByClass(TargetClass->GetClassPathName(), Assets, true);
#else
    AssetRegistry.GetAssetsByClass(TargetClass->GetFName(), Assets, true);
#endif

    for (const FAssetData &Asset : Assets)
    {
        UBlueprint *Instance = Cast<UBlueprint>(Asset.GetAsset());

        UE_LOG(LogTemp, Warning, TEXT("%s"), *Instance->GetName());

        for (const auto &Graph : Instance->UbergraphPages)
        {
            TArray<UK2Node_MacroInstance *> Nodes;
            Graph->GetNodesOfClass<UK2Node_MacroInstance>(Nodes);

            for (const auto &Node : Nodes)
            {
                UE_LOG(LogTemp, Warning, TEXT("- %s"), *Node->GetMacroGraph()->GetName());

                if (Node->GetMacroGraph()->GetName().Contains(TEXT("SpawnActor")))
                {
                    UE_LOG(LogTemp, Warning, TEXT("SpawnActor"));

                    for (const auto &Pin : Node->Pins)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("- %s"), *Pin->GetName());
                    }
                }
                
                if (false)
                {
                    if (Node->GetMacroGraph()->GetName() == TEXT("Create Task from User Widget") ||
                        Node->GetMacroGraph()->GetName() == TEXT("Create Task from Actor") ||
                        Node->GetMacroGraph()->GetName() == TEXT("Create Task from Game Instance"))
                    {
                        UEdGraphPin *TaskPin = nullptr;
                        UEdGraphPin *InstancePin = nullptr;
                        UEdGraphPin *ExecPin = nullptr;
                        UEdGraphPin *ReturnPin = nullptr;
                        for (const auto &Pin : Node->Pins)
                        {
                            if (Pin->GetName() == TEXT("Task"))
                            {
                                TaskPin = Pin;
                            }
                            if (Pin->GetName() == TEXT("Instance"))
                            {
                                InstancePin = Pin;
                            }
                            if (Pin->GetName() == TEXT("Exec"))
                            {
                                ExecPin = Pin;
                            }
                            if (Pin->GetName() == TEXT("Return"))
                            {
                                ReturnPin = Pin;
                            }
                        }

                        if (TaskPin == nullptr || InstancePin == nullptr || ReturnPin == nullptr || ExecPin == nullptr)
                        {
                            continue;
                        }

                        if (InstancePin->LinkedTo.Num() != 1)
                        {
                            // We only ever cast after making the task
                            // so this would be weird.
                            continue;
                        }

                        UK2Node_DynamicCast *CastNode =
                            Cast<UK2Node_DynamicCast>(InstancePin->LinkedTo[0]->GetOwningNode());
                        UEdGraphPin *CastResultPin = CastNode->GetCastResultPin();

                        UE_LOG(LogTemp, Warning, TEXT("- Exec: %s"), *ExecPin->GetName());
                        UE_LOG(LogTemp, Warning, TEXT("- Task: %s"), *TaskPin->GetName());
                        UE_LOG(LogTemp, Warning, TEXT("- Instance: %s"), *InstancePin->GetName());
                        UE_LOG(LogTemp, Warning, TEXT("- Return: %s"), *ReturnPin->GetName());

                        UBlueprintGeneratedClass *TaskClass = Cast<UBlueprintGeneratedClass>(TaskPin->DefaultObject);

                        UK2Node_SpawnActorFromClass *SpawnNode = NewObject<UK2Node_SpawnActorFromClass>( 
                            Graph,
                            UK2Node_SpawnActorFromClass::StaticClass(),
                            NAME_None,
                            RF_Transactional);
                        SpawnNode->AllocateDefaultPins();
                        Graph->AddNode(SpawnNode);

                        SpawnNode->NodePosX = Node->NodePosX;
                        SpawnNode->NodePosY = Node->NodePosY;
                        SpawnNode->GetClassPin()->DefaultObject = TaskPin->DefaultObject;
                        for (const auto &Pin : SpawnNode->Pins)
                        {
                        }

                        this->MovePinsAwayToAnotherPin(CastResultPin, SpawnNode->GetResultPin());
                        this->MovePinsAwayToAnotherPin(ExecPin, SpawnNode->GetExecPin());
                        this->MovePinsAwayToAnotherPin(ReturnPin, SpawnNode->GetThenPin());

                        UE_LOG(LogTemp, Warning, TEXT("- Task: %s"), *TaskClass->GetName());

                        this->SavePackageHelper(Instance->GetPackage());

                        return 0;
                    }
                }
            }
        }
    }

    return 0;
}

void UEraseTasksCommandlet::MovePinsAwayToAnotherPin(
    UEdGraphPin *CurrentPin,
    UEdGraphPin *NewPin)
{
    check(CurrentPin != nullptr);
    check(NewPin != nullptr);

    // Iterate through all the pins that the existing pin is linked to and move
    // them to the new result pin.
    TArray<UEdGraphPin *> LinkedPins = CurrentPin->LinkedTo;
    for (const auto &Connection : LinkedPins)
    {
        Connection->MakeLinkTo(NewPin);
        Connection->BreakLinkTo(CurrentPin);
    }
}

bool UEraseTasksCommandlet::SavePackageHelper(UPackage *Package)
{
    FString PackageFilename = USourceControlHelpers::PackageFilename(Package);

    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Standalone;
    SaveArgs.Error = GWarn;
    return GEditor->SavePackage(Package, nullptr, *PackageFilename, SaveArgs);
}