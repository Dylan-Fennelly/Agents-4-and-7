/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/
#pragma once
/**
 * @brief Represents the controller type.
 */
enum class ControllerType
{
	/// <summary>
	/// For Controllers that are similar to the Xbox controller and use the same button layout
	/// </summary>
	kXbox,
	/// <summary>
	/// For Controllers that are similar to the PS4 controller and use the same button layout
	/// </summary>
	kPs4,
	/// <summary>
	/// For unknown controllers, it will use the Xbox controller layout as a default but this may be subject to change
	/// </summary>
	kUnknown
};