package com.raphnet.tenki;

/**
 * Collection of static native methods for libtenki used internally.
 *
 * Not normally used outside com.raphnet.tenki
 *
 */
public class JNI
{
	/**
	 * Get converted channel value.
	 *
	 * readChannel must be called prior to using this method.
	 *
	 * @param id The channel id
	 * @param handle The native handle
	 * @return The channel value
	 */
	public static native float getConvertedChannelValue(long handle, int id);

	/**
	 * Get converted channel unit.
	 *
	 * readChannel must be called prior to using this method.
	 *
	 * @see com.raphnet.tenki.TenkiConstants
	 * @param id The channel id
	 * @param handle The native handle
	 * @return The converted channel unit.
	 */
	public static native int getConvertedChannelUnit(long handle, int id);

	/**
	 * Perform a read from the device.
	 *
	 * Data and related information will be stored in memory. Fetch
	 * them with other methods such as getConvertedChannelValue and getConvertedChannelUnit.
	 *
	 * @param handle The native handle
	 * @param id The channel id.
	 */
	public static native void readChannel(long handle, int id);

	/**
	 * Open an USB device by it's serial number.
	 *
	 * @param serial The case-sensitive serial number
	 * @return The native handle
	 */
	public static native long openBySerial(String serial);

	/**
	 * Get the device firmware version.
	 *
	 * @param handle The native handle.
	 * @return bits 15-8: Major, 7-0: Minor
	 */
	public static native int getVersion(long handle);

	/**
	 * Get the number of channels supported by device.
	 *
	 * This method returns the number of channels supported
	 * by a device. Channels are referred to by their ID, but those 
	 * IDs are not necessary consecutive. 
	 *
	 * @see #getChannelId(long, int)
	 * @param handle The native handle
	 * @return The number of supported channels.
	 */
	public static native int getNumChannels(long handle);

	/**
	 * Get the channel ID corresponding to an index.
	 *
	 * Channels are referred to by their ID, but those 
	 * IDs are not necessary consecutive. You can specify an
	 * index to this method to obtain the channel ID. (eg, 3rd channel, index 2)

	 * @param handle The native handle
	 * @param index The index
	 */
	public static native int getChannelId(long handle, int index);

	/**
	 * Get the name of the channel.
	 *
	 * This will return something like this:
	 * <pre>
	 * "Sensirion SHT1x/7x Temperature"
	 * </pre>
	 *
	 * @param handle The native handle
	 * @param id The channel ID
	 * @return The channel name
	 */
	public static native String getChannelName(long handle, int id);

	/**
	 * Get the chip id of the channel.
	 *
	 * The returned chip id will be (probably) one of the CHIP_xx constants
	 * from the TenkiConstans class.
	 *
	 * @param handle The native handle
	 * @param id The channel id
	 * @return The chip id.
	 * @see com.raphnet.tenki.TenkiConstants
	 */
	public static native int getChannelChipId(long handle, int id);

	/**
	 * Return a short name for the channel.
	 *
	 * This will return something like this:
	 * <pre>
	 * Temperature
	 * </pre>
	 *
	 * @param handle The native handle
	 * @param id The channel ID
	 * @return The channel name
	 */
	public static native String getChannelTypeName(long handle, int id);

	/**
	 * Return a name for a unit type.
	 *
	 * @param unit The unit type
	 * @see com.raphnet.tenki.TenkiConstants
	 * @return The name of the unit type.
	 */
	public static native String unitToName(int unit);

	/**
	 * Get the name of the chip id.
	 *
	 * @param chip The chip id
	 * @see com.raphnet.tenki.TenkiConstants
	 * @return The name of the chip id.
	 */
	public static native String chipToName(int chip);

	/**
	 * Clean up (close) the device.
	 *
	 * The handle will be invalid afterwards!
	 *
	 * @param handle The native handle.
	 */
	public static native void cleanUp(long handle);


	public static native String getDeviceList();

	/**
	 * Initialize libusb.
	 *
	 * Libusb must be initialized before attempting to use
	 * it's functionalities. This method is automatically called
	 * from this class 'static' block, after the load library.
	 */
	public static native void initLibusb();

	static {		
		System.loadLibrary("tenki");
		initLibusb();
	}

}
