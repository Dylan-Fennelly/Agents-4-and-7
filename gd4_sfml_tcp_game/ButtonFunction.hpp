#pragma once
/// <summary>
/// Represents the function of a button on a gamepad
/// </summary>
enum class ButtonFunction
{
	/// <summary>
	/// Comfirmation button or bacl button for menus
	/// </summary>
	kConfirm,
	/// <summary>
	/// Cancellation or back button for menus
	/// </summary>
	kCancel,
	/// <summary>
	/// Button for pausing the game
	/// </summary>
	kPause,
	/// <summary>
	/// The number of buttons we have assigned a gamepad function to
	/// </summary>
	kButtonCount
};