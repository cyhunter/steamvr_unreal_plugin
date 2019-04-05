/*
Copyright 2019 Valve Corporation under https://opensource.org/licenses/BSD-3-Clause

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this 
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation 
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors 
   may be used to endorse or promote products derived from this software 
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include "SteamVREditor.h"
#include "IMotionController.h"
#include "Features/IModularFeatures.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "SteamVREditorStyle.h"
#include "SteamVREditorCommands.h"
#include "SteamVRControllerKeys.h"
#include "LevelEditor.h"

static const FName SteamVREditorTabName("SteamVREditor");

#define LOCTEXT_NAMESPACE "FSteamVREditorModule"

void FSteamVREditorModule::StartupModule()
{
	FSteamVREditorStyle::Initialize();
	FSteamVREditorStyle::ReloadTextures();

	FSteamVREditorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	// Dummy action for main toolbar button
	PluginCommands->MapAction(
		FSteamVREditorCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FSteamVREditorModule::PluginButtonClicked),
		FCanExecuteAction());
	
	// Regenerate Action Manifest
	PluginCommands->MapAction(
		FSteamVREditorCommands::Get().JsonActionManifest,
		FExecuteAction::CreateRaw(this, &FSteamVREditorModule::JsonRegenerateActionManifest),
		FCanExecuteAction());

	// Regenerate Controller Bindings
	PluginCommands->MapAction(
		FSteamVREditorCommands::Get().JsonControllerBindings,
		FExecuteAction::CreateRaw(this, &FSteamVREditorModule::JsonRegenerateControllerBindings),
		FCanExecuteAction());

	// Reload Action Manifest
	PluginCommands->MapAction(
		FSteamVREditorCommands::Get().ReloadActionManifest,
		FExecuteAction::CreateRaw(this, &FSteamVREditorModule::ReloadActionManifest),
		FCanExecuteAction());

	// Launch Bindings URL
	PluginCommands->MapAction(
		FSteamVREditorCommands::Get().LaunchBindingsURL,
		FExecuteAction::CreateRaw(this, &FSteamVREditorModule::LaunchBindingsURL),
		FCanExecuteAction());
	
	// Add Sample Inputs
	PluginCommands->MapAction(
		FSteamVREditorCommands::Get().AddSampleInputs,
		FExecuteAction::CreateRaw(this, &FSteamVREditorModule::AddSampleInputs),
		FCanExecuteAction());

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FSteamVREditorModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FSteamVREditorModule::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
}

void FSteamVREditorModule::ShutdownModule()
{
	FSteamVREditorStyle::Shutdown();

	FSteamVREditorCommands::Unregister();
}

void FSteamVREditorModule::PluginButtonClicked()
{
	// Empty on purpose
}

void FSteamVREditorModule::JsonRegenerateActionManifest()
{
	USteamVRInputDeviceFunctionLibrary::RegenActionManifest();
}

void FSteamVREditorModule::JsonRegenerateControllerBindings()
{
	USteamVRInputDeviceFunctionLibrary::RegenControllerBindings();
}

void FSteamVREditorModule::ReloadActionManifest()
{
	USteamVRInputDeviceFunctionLibrary::ReloadActionManifest();
}

void FSteamVREditorModule::LaunchBindingsURL()
{
	USteamVRInputDeviceFunctionLibrary::LaunchBindingsURL();
}

void FSteamVREditorModule::AddSampleInputs()
{
	// Get Existing Input Settings
	auto DefaultInputSettings = GetDefault<UInputSettings>();
	TArray<FInputAxisKeyMapping> ExistingAxisKeys = DefaultInputSettings->AxisMappings;
	TArray<FInputActionKeyMapping> ExistingActionKeys = DefaultInputSettings->ActionMappings;

	// Create new Input Settings
	UInputSettings* InputSettings = GetMutableDefault<UInputSettings>();

	if (InputSettings->IsValidLowLevel())
	{
		// Sample analog mapping
		AddUniqueAxisMapping(ExistingAxisKeys, InputSettings, FName(TEXT("SteamVR Test Analog Mapping")), EKeys::MotionController_Left_TriggerAxis);
		
		// Sample digital mapping
		AddUniqueActionMapping(ExistingActionKeys, InputSettings, FName(TEXT("SteamVR Test Digital Mapping")), EKeys::MotionController_Left_Trigger);

		// Sample mapping - custom knuckles keys
		AddUniqueActionMapping(ExistingActionKeys, InputSettings, FName(TEXT("SteamVR Test Knuckles Mapping")), IndexControllerKeys::SteamVR_Valve_Index_Controller_Pinch_Grab_Left);

		// Update the config file
		InputSettings->SaveKeyMappings();
		InputSettings->UpdateDefaultConfigFile();
	}
}

bool FSteamVREditorModule::AddUniqueAxisMapping(TArray<FInputAxisKeyMapping> ExistingAxisKeys, UInputSettings* InputSettings, FName ActionName, FKey ActionKey)
{
	// Create new axis mapping
	FInputAxisKeyMapping NewAxisMapping = FInputAxisKeyMapping(ActionName, ActionKey);

	// Check if this mapping already exists in the project
	if (ExistingAxisKeys.Find(NewAxisMapping) < 1)
	{
		// If none, create a new one
		InputSettings->AddAxisMapping(NewAxisMapping);
		return true;
	}
	
return false;
}

bool FSteamVREditorModule::AddUniqueActionMapping(TArray<FInputActionKeyMapping> ExistingActionKeys, UInputSettings* InputSettings, FName ActionName, FKey ActionKey)
{
	// Create new action mapping
	FInputActionKeyMapping NewActionMapping = FInputActionKeyMapping(ActionName, ActionKey);

	// Check if this mapping already exists in the project
	if (ExistingActionKeys.Find(NewActionMapping) < 1)
	{
		// If none, create a new one
		InputSettings->AddActionMapping(NewActionMapping);
		return true;
	}

	return false;
}

void FSteamVREditorModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FSteamVREditorCommands::Get().PluginAction);
}

void FSteamVREditorModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	FSteamVREditorStyle MenuStyle = FSteamVREditorStyle();
	MenuStyle.Initialize();

	Builder.AddComboButton(
		FUIAction(FExecuteAction::CreateRaw(this, &FSteamVREditorModule::PluginButtonClicked)),
		FOnGetContent::CreateRaw(this, &FSteamVREditorModule::FillComboButton, PluginCommands),
		LOCTEXT("SteamVRInputBtn", "SteamVR Input"),
		LOCTEXT("SteamVRInputBtnTootlip", "SteamVR Input"),
		FSlateIcon(FSteamVREditorStyle::GetStyleSetName(), "SteamVREditor.PluginAction")
	);
}

TSharedRef<SWidget> FSteamVREditorModule::FillComboButton(TSharedPtr<class FUICommandList> Commands)
{
	FMenuBuilder MenuBuilder(true, Commands);

	MenuBuilder.AddMenuEntry(FSteamVREditorCommands::Get().JsonActionManifest, NAME_None, TAttribute<FText>(), TAttribute<FText>(), FSlateIcon(FSteamVREditorStyle::GetStyleSetName(), "SteamVREditor.JsonActionManifest"));
	MenuBuilder.AddMenuEntry(FSteamVREditorCommands::Get().JsonControllerBindings, NAME_None, TAttribute<FText>(), TAttribute<FText>(), FSlateIcon(FSteamVREditorStyle::GetStyleSetName(), "SteamVREditor.JsonControllerBindings"));
	MenuBuilder.AddMenuEntry(FSteamVREditorCommands::Get().ReloadActionManifest, NAME_None, TAttribute<FText>(), TAttribute<FText>(), FSlateIcon(FSteamVREditorStyle::GetStyleSetName(), "SteamVREditor.ReloadActionManifest"));
	MenuBuilder.AddMenuEntry(FSteamVREditorCommands::Get().LaunchBindingsURL, NAME_None, TAttribute<FText>(), TAttribute<FText>(), FSlateIcon(FSteamVREditorStyle::GetStyleSetName(), "SteamVREditor.LaunchBindingsURL"));
	MenuBuilder.AddMenuEntry(FSteamVREditorCommands::Get().AddSampleInputs, NAME_None, TAttribute<FText>(), TAttribute<FText>(), FSlateIcon(FSteamVREditorStyle::GetStyleSetName(), "SteamVREditor.AddSampleInputs"));

	return MenuBuilder.MakeWidget();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSteamVREditorModule, SteamVREditor)