package com.raphnet.tenki;


/**
 * Interface for TenkiDevice of all types.
 */
public interface TenkiDevice {

	
	/* Get the type of device.
	 *
	 * USB, Serial, etc
	 */
	public String getType();

	/* Get the device's serial number.
	 */
	public String getSerial();

	/* Get the device's firmware version
	 */
	public String getVersion();

	/* Get the list of channels available 
	 */
	public TenkiDeviceChannel[] getAvailableChannels();
}
