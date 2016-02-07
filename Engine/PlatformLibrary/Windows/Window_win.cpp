#include "Window_win.h"

#include <cassert>
#include <limits>
#include <assert.h>
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/RenderTexture.hpp"

Window::Window(const rWindow& d)
{
	lastMousePos.x = std::numeric_limits<int>::min();
	lastMousePos.y = std::numeric_limits<int>::min();

	sf::Uint32 windowStyle;
	// Client size == Screen size, we must use FullScreen style
	//if (d.clientSize == Sys::GetScreenSize()) waiting for mymath improvement
	if (d.clientSize.x == Sys::GetScreenSize().x && d.clientSize.y == Sys::GetScreenSize().y)
		windowStyle = ConvertToSFMLWindowStyle(eWindowStyle::BORDERLESS);
	else
		windowStyle = ConvertToSFMLWindowStyle(d.style);

	w.create(sf::VideoMode(d.clientSize.x, d.clientSize.y), d.capText.c_str(), windowStyle);
	//w.setVerticalSyncEnabled(true);
}

Window::~Window()
{
	w.close();
}

bool Window::PopEvent(rWindowEvent& evt_out)
{
	sf::Event evt;
	if (!w.pollEvent(evt))
		return false;

	// Key press release,
	if (evt.type == sf::Event::EventType::KeyPressed || evt.type == sf::Event::EventType::KeyReleased) 
	{
		evt_out.key = ConvertFromSFMLKey(evt.key.code);
	}
	else if (evt.type == sf::Event::EventType::MouseMoved) 
	{
		// TODO: worst idea ever
		if (lastMousePos.x == std::numeric_limits<int>::min()) 
		{
			lastMousePos.x = evt.mouseMove.x;
			lastMousePos.y = evt.mouseMove.y;
		}

		evt_out.deltaX = evt.mouseMove.x - lastMousePos.x;
		evt_out.deltaY = evt.mouseMove.y - lastMousePos.y;
		
		evt_out.x = evt.mouseMove.x;
		evt_out.y = evt.mouseMove.y;

		lastMousePos.x = evt.mouseMove.x;
		lastMousePos.y = evt.mouseMove.y;
	}
	else if (evt.type == sf::Event::EventType::MouseButtonPressed || evt.type == sf::Event::EventType::MouseButtonReleased)	
	{
		evt_out.x = evt.mouseButton.x;
		evt_out.y = evt.mouseButton.y;
		evt_out.mouseBtn = ConvertFromSFMLMouseBtn(evt.mouseButton.button);
	}
	else if (evt.type == sf::Event::EventType::MouseWheelMoved)
	{
		evt_out.deltaY = evt.mouseWheel.delta;
	}
	else if (evt.type == sf::Event::EventType::Closed)
	{
		Close();
	}
	else if (evt.type == sf::Event::EventType::Resized)
	{
		evt_out.x = evt.size.width;
		evt_out.y = evt.size.height;
	}

	evt_out.msg = ConvertFromSFMLWindowMsg(evt.type);

	return true;
}

void Window::Close() 
{
	w.close();
}

void Window::Clear(const Color& color)
{
	w.clear(sf::Color(color.r, color.g, color.b, color.a));
}

void Window::Present() 
{
	w.display();
}

void Window::SetPos(const mm::ivec2& pos /*= mm::ivec2(0, 0)*/)
{
	w.setPosition(sf::Vector2i(pos.x, pos.y));
}

void Window::SetSize(const mm::uvec2& pos)
{
	w.setSize(sf::Vector2u(pos.x, pos.y));
}

void Window::SetClientPixels(const Color* const pixels)
{
	static thread_local sf::Sprite	sprite;
	static thread_local sf::Texture texture;
	static thread_local bool bInited = false;

	if (!bInited)
	{
		texture.create(GetClientWidth(), GetClientHeight());
		sprite.setTexture(texture);
		bInited = true;
	}

	texture.update((uint8_t*)pixels);

	w.draw(sprite);
}

void Window::SetCursorVisible(bool bVisible)
{
	w.setMouseCursorVisible(bVisible);
}

void Window::SetTitle(const std::string& text)
{
	w.setTitle(text);
}

bool Window::IsOpen() const
{
	return w.isOpen();
}

size_t Window::GetHandle() const
{
	return (size_t)w.getSystemHandle();
}

u16 Window::GetClientWidth() const 
{
	return w.getSize().x;
}

u16 Window::GetClientHeight() const 
{
	return w.getSize().y;
}

unsigned Window::GetNumClientPixels() const
{
	return w.getSize().x * w.getSize().y;
}

float Window::GetClientAspectRatio() const 
{
	const sf::Vector2u size = w.getSize();
	return (float)size.x / size.y;
}

mm::vec2 Window::GetCenterPos() const
{
	auto size = w.getSize();
	auto pos = w.getPosition();

	return mm::vec2(pos.x + size.x * 0.5, pos.y + size.y * 0.5);
}

eWindowMsg Window::ConvertFromSFMLWindowMsg(sf::Event::EventType windowMsg)
{
	switch (windowMsg)
	{
	case sf::Event::EventType::Closed:					return eWindowMsg::CLOSE;
	case sf::Event::EventType::Resized:					return eWindowMsg::RESIZE;
	case sf::Event::EventType::LostFocus:				return eWindowMsg::DEFOCUS;
	case sf::Event::EventType::GainedFocus:				return eWindowMsg::FOCUS;
	case sf::Event::EventType::TextEntered:				return eWindowMsg::TEXT_ENTERED;
	case sf::Event::EventType::KeyPressed:				return eWindowMsg::KEY_PRESS;
	case sf::Event::EventType::KeyReleased:				return eWindowMsg::KEY_RELEASE;
	case sf::Event::EventType::MouseWheelScrolled:
	case sf::Event::EventType::MouseWheelMoved:			return eWindowMsg::MOUSE_SCROLL;
	case sf::Event::EventType::MouseButtonPressed:		return eWindowMsg::MOUSE_PRESS;
	case sf::Event::EventType::MouseButtonReleased:		return eWindowMsg::MOUSE_RELEASE;
	case sf::Event::EventType::MouseMoved:				return eWindowMsg::MOUSE_MOVE;
	case sf::Event::EventType::MouseEntered:			return eWindowMsg::MOUSE_ENTER;
	case sf::Event::EventType::MouseLeft:				return eWindowMsg::MOUSE_LEAVE;
	case sf::Event::EventType::JoystickButtonPressed:	return eWindowMsg::JOYSTICK_BUTTON_PRESS;
	case sf::Event::EventType::JoystickButtonReleased:	return eWindowMsg::JOYSTICK_BUTTON_RELEASE;
	case sf::Event::EventType::JoystickMoved:			return eWindowMsg::JOYSTICK_MOVE;
	case sf::Event::EventType::JoystickConnected:		return eWindowMsg::JOYSTICK_CONNECT;
	case sf::Event::EventType::JoystickDisconnected:	return eWindowMsg::JOYSTICK_DISCONNECT;
	}

	return eWindowMsg::INVALID;
}

eMouseBtn Window::ConvertFromSFMLMouseBtn(sf::Mouse::Button btn)
{
	switch (btn)
	{
		case sf::Mouse::Button::Left:		return eMouseBtn::LEFT;
		case sf::Mouse::Button::Right:		return eMouseBtn::RIGHT;
		case sf::Mouse::Button::Middle:		return eMouseBtn::MID;
		case sf::Mouse::Button::XButton1:	return eMouseBtn::EXTRA1;
		case sf::Mouse::Button::XButton2:	return eMouseBtn::EXTRA2;
	}

	return eMouseBtn::INVALID;
}

eKey Window::ConvertFromSFMLKey(sf::Keyboard::Key key)
{
	switch (key)
	{
	case sf::Keyboard::A:			return eKey::A;
	case sf::Keyboard::B:			return eKey::B;
	case sf::Keyboard::C:			return eKey::C;
	case sf::Keyboard::D:			return eKey::D;
	case sf::Keyboard::E:			return eKey::E;
	case sf::Keyboard::F:			return eKey::F;
	case sf::Keyboard::G:			return eKey::G;
	case sf::Keyboard::H:			return eKey::H;
	case sf::Keyboard::I:			return eKey::I;
	case sf::Keyboard::J:			return eKey::J;
	case sf::Keyboard::K:			return eKey::K;
	case sf::Keyboard::L:			return eKey::L;
	case sf::Keyboard::M:			return eKey::M;
	case sf::Keyboard::N:			return eKey::N;
	case sf::Keyboard::O:			return eKey::O;
	case sf::Keyboard::P:			return eKey::P;
	case sf::Keyboard::Q:			return eKey::Q;
	case sf::Keyboard::R:			return eKey::R;
	case sf::Keyboard::S:			return eKey::S;
	case sf::Keyboard::T:			return eKey::T;
	case sf::Keyboard::U:			return eKey::U;
	case sf::Keyboard::V:			return eKey::V;
	case sf::Keyboard::W:			return eKey::W;
	case sf::Keyboard::X:			return eKey::X;
	case sf::Keyboard::Y:			return eKey::Y;
	case sf::Keyboard::Z:			return eKey::Z;
	case sf::Keyboard::Num0:		return eKey::NUM0;
	case sf::Keyboard::Num1:		return eKey::NUM1;
	case sf::Keyboard::Num2:		return eKey::NUM2;
	case sf::Keyboard::Num3:		return eKey::NUM3;
	case sf::Keyboard::Num4:		return eKey::NUM4;
	case sf::Keyboard::Num5:		return eKey::NUM5;
	case sf::Keyboard::Num6:		return eKey::NUM6;
	case sf::Keyboard::Num7:		return eKey::NUM7;
	case sf::Keyboard::Num8:		return eKey::NUM8;
	case sf::Keyboard::Num9:		return eKey::NUM9;
	case sf::Keyboard::Escape:		return eKey::ESC;
	case sf::Keyboard::LControl:	return eKey::LCTRL;
	case sf::Keyboard::LShift:		return eKey::LSHIFT;
	case sf::Keyboard::LAlt:		return eKey::LALT;
	case sf::Keyboard::LSystem:		return eKey::LSYS;
	case sf::Keyboard::RControl:	return eKey::RCTRL;
	case sf::Keyboard::RShift:		return eKey::RSHIFT;
	case sf::Keyboard::RAlt:		return eKey::RALT;
	case sf::Keyboard::RSystem:		return eKey::RSYS;
	case sf::Keyboard::Menu:		return eKey::MENU;
	case sf::Keyboard::LBracket:	return eKey::LBRACKET;
	case sf::Keyboard::RBracket:	return eKey::RBRACKET;
	case sf::Keyboard::SemiColon:	return eKey::SEMICOLON;
	case sf::Keyboard::Comma:		return eKey::COMMA;
	case sf::Keyboard::Period:		return eKey::PERIOD;
	case sf::Keyboard::Quote:		return eKey::QUOTE;
	case sf::Keyboard::Slash:		return eKey::SLASH;
	case sf::Keyboard::BackSlash:	return eKey::BACKSLASH;
	case sf::Keyboard::Tilde:		return eKey::TILDE;
	case sf::Keyboard::Equal:		return eKey::EQUAL;
	case sf::Keyboard::Dash:		return eKey::DASH;
	case sf::Keyboard::Space:		return eKey::SPACE;
	case sf::Keyboard::Return:		return eKey::RETURN;
	case sf::Keyboard::BackSpace:	return eKey::BACKSPACE;
	case sf::Keyboard::Tab:			return eKey::TAB;
	case sf::Keyboard::PageUp:		return eKey::PAGEUP;
	case sf::Keyboard::PageDown:	return eKey::PAGEDDOWN;
	case sf::Keyboard::End:			return eKey::END;
	case sf::Keyboard::Home:		return eKey::HOME;
	case sf::Keyboard::Insert:		return eKey::INS;
	case sf::Keyboard::Delete:		return eKey::DEL;
	case sf::Keyboard::Add:			return eKey::ADD;
	case sf::Keyboard::Subtract:	return eKey::SUB;
	case sf::Keyboard::Multiply:	return eKey::MUL;
	case sf::Keyboard::Divide:		return eKey::DIV;
	case sf::Keyboard::Left:		return eKey::LEFT;
	case sf::Keyboard::Right:		return eKey::RIGHT;
	case sf::Keyboard::Up:			return eKey::UP;
	case sf::Keyboard::Down:		return eKey::DOWN;
	case sf::Keyboard::Numpad0:		return eKey::NUM0;
	case sf::Keyboard::Numpad1:		return eKey::NUM1;
	case sf::Keyboard::Numpad2:		return eKey::NUM2;
	case sf::Keyboard::Numpad3:		return eKey::NUM3;
	case sf::Keyboard::Numpad4:		return eKey::NUM4;
	case sf::Keyboard::Numpad5:		return eKey::NUM5;
	case sf::Keyboard::Numpad6:		return eKey::NUM6;
	case sf::Keyboard::Numpad7:		return eKey::NUM7;
	case sf::Keyboard::Numpad8:		return eKey::NUM8;
	case sf::Keyboard::Numpad9:		return eKey::NUM9;
	case sf::Keyboard::F1:			return eKey::F1;
	case sf::Keyboard::F2:			return eKey::F2;
	case sf::Keyboard::F3:			return eKey::F3;
	case sf::Keyboard::F4:			return eKey::F4;
	case sf::Keyboard::F5:			return eKey::F5;
	case sf::Keyboard::F6:			return eKey::F6;
	case sf::Keyboard::F7:			return eKey::F7;
	case sf::Keyboard::F8:			return eKey::F8;
	case sf::Keyboard::F9:			return eKey::F9;
	case sf::Keyboard::F10:			return eKey::F10;
	case sf::Keyboard::F11:			return eKey::F11;
	case sf::Keyboard::F12:			return eKey::F12;
	case sf::Keyboard::F13:			return eKey::F13;
	case sf::Keyboard::F14:			return eKey::F14;
	case sf::Keyboard::F15:			return eKey::F15;
	case sf::Keyboard::Pause:		return eKey::PAUSE;
	case sf::Keyboard::KeyCount:	return eKey::COUNT;
	}

	return eKey::INVALID;
}

sf::Uint32	Window::ConvertToSFMLWindowStyle(eWindowStyle style)
{
	switch (style)
	{
	case eWindowStyle::DEFAULT:							return sf::Style::Default;
	case eWindowStyle::BORDERLESS:						return sf::Style::None;
	case eWindowStyle::TITLE_FIXBORDER:					return sf::Style::Titlebar;
	case eWindowStyle::TITLE_RESIZEABLEBORDER:			return sf::Style::Titlebar | sf::Style::Resize;
	case eWindowStyle::TITLE_CLOSEABLE:					return sf::Style::Titlebar | sf::Style::Close;
	}

	return sf::Style::Default;
}