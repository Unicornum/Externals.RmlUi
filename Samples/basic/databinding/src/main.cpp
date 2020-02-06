﻿/*
 * This source file is part of RmlUi, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://github.com/mikke89/RmlUi
 *
 * Copyright (c) 2018 Michael R. P. Ragazzon
 * Copyright (c) 2019 The RmlUi Team, and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <RmlUi/Core.h>
#include <RmlUi/Controls.h>
#include <RmlUi/Debugger.h>
#include <Input.h>
#include <Shell.h>
#include <ShellRenderInterfaceOpenGL.h>
#include <numeric>


class DemoWindow : public Rml::Core::EventListener
{
public:
	DemoWindow(const Rml::Core::String &title, const Rml::Core::Vector2f &position, Rml::Core::Context *context)
	{
		using namespace Rml::Core;
		document = context->LoadDocument("basic/databinding/data/databinding.rml");
		if (document)
		{
			document->GetElementById("title")->SetInnerRML(title);
			document->SetProperty(PropertyId::Left, Property(position.x, Property::PX));
			document->SetProperty(PropertyId::Top, Property(position.y, Property::PX));

			document->Show();
		}
	}

	void Update() 
	{

	}

	void Shutdown() 
	{
		if (document)
		{
			document->Close();
			document = nullptr;
		}
	}

	void ProcessEvent(Rml::Core::Event& event) override
	{
		using namespace Rml::Core;

		switch (event.GetId())
		{
		case EventId::Keydown:
		{
			Rml::Core::Input::KeyIdentifier key_identifier = (Rml::Core::Input::KeyIdentifier) event.GetParameter< int >("key_identifier", 0);
			bool ctrl_key = event.GetParameter< bool >("ctrl_key", false);

			if (key_identifier == Rml::Core::Input::KI_ESCAPE)
			{
				Shell::RequestExit();
			}
			else if (key_identifier == Rml::Core::Input::KI_F8)
			{
				Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());
			}
		}
		break;

		default:
			break;
		}
	}

	Rml::Core::ElementDocument * GetDocument() {
		return document;
	}


private:
	Rml::Core::ElementDocument *document = nullptr;
};



struct Invader {
	Rml::Core::String name;
	Rml::Core::String sprite;
	Rml::Core::Colourb color{ 255, 255, 255 };
	std::vector<int> numbers = { 1, 2, 3, 4, 5 };

	void GetColor(Rml::Core::Variant& variant) {
		variant = "rgba(" + Rml::Core::ToString(color) + ')';
	}

	void OnClick(Rml::Core::DataModelHandle model_handle, Rml::Core::Event& ev, const Rml::Core::VariantList& arguments)
	{
		//ev.GetCurrentElement()->GetDataModel()->
		//model_handle.DirtyVariable("invaders");
	}
};


struct MyData {
	Rml::Core::String hello_world = "Hello World!";
	int rating = 99;


	Invader delightful_invader{ "Delightful invader", "icon-invader" };

	std::vector<Invader> invaders = {
		Invader{"Angry invader", "icon-invader", {255, 40, 30}, {3, 6, 7}},
		Invader{"Harmless invader", "icon-flag", {20, 40, 255}, {5, 0}},
		Invader{"Hero", "icon-game", {255, 255, 30}, {10, 11, 12, 13, 14}},
	};

	std::vector<int> indices = { 1, 2, 3, 4, 5 };
} my_data;

Rml::Core::DataModelHandle my_model;


void HasGoodRating(Rml::Core::Variant& variant) {
	variant = int(my_data.rating > 50);
}

bool SetupDataBinding(Rml::Core::Context* context)
{
	my_model = context->CreateDataModel("my_model");
	if (!my_model)
		return false;

	my_model.Bind("hello_world", &my_data.hello_world);
	my_model.Bind("rating", &my_data.rating);
	my_model.BindFunc("good_rating", &HasGoodRating);
	my_model.BindFunc("great_rating", [](Rml::Core::Variant& variant) {
		variant = int(my_data.rating > 80);
	});

	my_model.RegisterArray<std::vector<int>>();

	if(auto invader_handle = my_model.RegisterStruct<Invader>())
	{
		invader_handle.AddMember("name", &Invader::name);
		invader_handle.AddMember("sprite", &Invader::sprite);
		invader_handle.AddMember("numbers", &Invader::numbers);
		invader_handle.AddMemberFunc("color", &Invader::GetColor);
	}

	my_model.Bind("delightful_invader", &my_data.delightful_invader);

	my_model.RegisterArray<std::vector<Invader>>();

	my_model.Bind("indices", &my_data.indices);
	my_model.Bind("invaders", &my_data.invaders);

	return true;
}



Rml::Core::Context* context = nullptr;
ShellRenderInterfaceExtensions *shell_renderer;
std::unique_ptr<DemoWindow> demo_window;

void GameLoop()
{
	if (my_model.IsVariableDirty("rating"))
	{
		my_model.DirtyVariable("good_rating");
		my_model.DirtyVariable("great_rating");

		size_t new_size = my_data.rating / 10 + 1;
		if (new_size != my_data.indices.size())
		{
			my_data.indices.resize(new_size);
			std::iota(my_data.indices.begin(), my_data.indices.end(), int(new_size));
			my_model.DirtyVariable("indices");
		}
	}

	my_model.Update();

	demo_window->Update();
	context->Update();

	shell_renderer->PrepareRenderBuffer();
	context->Render();
	shell_renderer->PresentRenderBuffer();
}




class DemoEventListener : public Rml::Core::EventListener
{
public:
	DemoEventListener(const Rml::Core::String& value, Rml::Core::Element* element) : value(value), element(element) {}

	void ProcessEvent(Rml::Core::Event& event) override
	{
		using namespace Rml::Core;

		if (value == "exit")
		{
			Shell::RequestExit();
		}
	}

	void OnDetach(Rml::Core::Element* element) override { delete this; }

private:
	Rml::Core::String value;
	Rml::Core::Element* element;
};



class DemoEventListenerInstancer : public Rml::Core::EventListenerInstancer
{
public:
	Rml::Core::EventListener* InstanceEventListener(const Rml::Core::String& value, Rml::Core::Element* element) override
	{
		return new DemoEventListener(value, element);
	}
};


#if defined RMLUI_PLATFORM_WIN32
#include <windows.h>
int APIENTRY WinMain(HINSTANCE RMLUI_UNUSED_PARAMETER(instance_handle), HINSTANCE RMLUI_UNUSED_PARAMETER(previous_instance_handle), char* RMLUI_UNUSED_PARAMETER(command_line), int RMLUI_UNUSED_PARAMETER(command_show))
#else
int main(int RMLUI_UNUSED_PARAMETER(argc), char** RMLUI_UNUSED_PARAMETER(argv))
#endif
{
#ifdef RMLUI_PLATFORM_WIN32
	RMLUI_UNUSED(instance_handle);
	RMLUI_UNUSED(previous_instance_handle);
	RMLUI_UNUSED(command_line);
	RMLUI_UNUSED(command_show);
#else
	RMLUI_UNUSED(argc);
	RMLUI_UNUSED(argv);
#endif

	const int width = 1600;
	const int height = 900;

	ShellRenderInterfaceOpenGL opengl_renderer;
	shell_renderer = &opengl_renderer;

	// Generic OS initialisation, creates a window and attaches OpenGL.
	if (!Shell::Initialise() ||
		!Shell::OpenWindow("Data Binding Sample", shell_renderer, width, height, true))
	{
		Shell::Shutdown();
		return -1;
	}

	// RmlUi initialisation.
	Rml::Core::SetRenderInterface(&opengl_renderer);
	opengl_renderer.SetViewport(width, height);

	ShellSystemInterface system_interface;
	Rml::Core::SetSystemInterface(&system_interface);

	Rml::Core::Initialise();

	// Create the main RmlUi context and set it on the shell's input layer.
	context = Rml::Core::CreateContext("main", Rml::Core::Vector2i(width, height));

	if (!context || !SetupDataBinding(context))
	{
		Rml::Core::Shutdown();
		Shell::Shutdown();
		return -1;
	}

	Rml::Controls::Initialise();
	Rml::Debugger::Initialise(context);
	Input::SetContext(context);
	shell_renderer->SetContext(context);
	
	DemoEventListenerInstancer event_listener_instancer;
	Rml::Core::Factory::RegisterEventListenerInstancer(&event_listener_instancer);

	Shell::LoadFonts("assets/");

	demo_window = std::make_unique<DemoWindow>("Data binding", Rml::Core::Vector2f(150, 50), context);
	demo_window->GetDocument()->AddEventListener(Rml::Core::EventId::Keydown, demo_window.get());
	demo_window->GetDocument()->AddEventListener(Rml::Core::EventId::Keyup, demo_window.get());

	Shell::EventLoop(GameLoop);

	demo_window->Shutdown();

	// Shutdown RmlUi.
	Rml::Core::Shutdown();

	Shell::CloseWindow();
	Shell::Shutdown();

	demo_window.reset();

	return 0;
}
