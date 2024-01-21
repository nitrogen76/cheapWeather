package com.raphnet.tenki;

/**
 * Class for using USBTenki devices.
 */
public class USBTenkiDevice implements TenkiDevice, TenkiChannelDataSource
{
	private long _native_pointer;
	private String _serial;
	private TenkiDeviceChannel[] _channels;
	private boolean show_unused_channels = false;

	/**
	 * Create an USBTenkiDevice instance for a specific serial.
	 *
	 * This creates an USBTenkiDevice instance meant to use an
	 * USB device with a specific serial number when {@link #open()} is called.
	 *
	 * @see #open()
	 * @param serial The case-sensitive serial number
	 */
	public USBTenkiDevice(String serial)
	{
		_serial = serial;
		_native_pointer = 0;
	}

	/**
	 * Open the USBTenkiDevice.
	 *
	 * Must be done before using most methods.
	 *
	 * @throws TenkiException if open fails.
	 */
	public void open() throws TenkiException
	{
		int nchn;

		_native_pointer = JNI.openBySerial(_serial);
		if (_native_pointer == 0) {
			throw new TenkiDeviceNotFoundException(_serial);
		}

		nchn = JNI.getNumChannels(_native_pointer);
		_channels = new TenkiDeviceChannel[nchn];

		for (int i=0; i<nchn; i++)
		{
			/* Retreive channel ID from list index */
			int id = JNI.getChannelId(_native_pointer, i);
			int chip_id;

			_channels[i] = new TenkiDeviceChannel(this,
						id,
						JNI.getChannelName(_native_pointer, id),
						JNI.getChannelChipId(_native_pointer, id),
						JNI.getChannelTypeName(_native_pointer, id));
		}

	}

	/**
	 * Get the type of device.
	 *
	 * @return This returns "USB"
	 */
	public String getType() { 
		return "USB";
	}

	/**
	 * Get the serial number of the device we are using (or will use).
	 *
	 * {@link #open()} may need to have been called for this to work.
	 *
	 * @return The serial number
	 */
	public String getSerial() {
		return _serial;
	}

	/**
	 * Get the version of the device's firmware.
	 *
	 * {@link #open()} must have been called for this to work.
	 * 
	 * @return The firmware version.
	 */
	public String getVersion() {
		int v = JNI.getVersion(_native_pointer);
		return new String("" + ((v&0xff00) >> 8) + "." + (v & 0xff));
	}

	/**
	 * Return the array of channels available.
	 *
	 * When {@link #open()} is called, the list of channels is read
	 * from the device and TenkiDeviceChannels are instantiated. The
	 * same instances are returned every time this method
	 * is called.
	 *
	 * @see TenkiDeviceChannel
	 * @return The array of channels.
	 */
	public TenkiDeviceChannel[] getAvailableChannels()
	{
		return _channels;
	}

	/**
	 * Call JNI cleanup if it was not already done manually.
	 */
	public void finalize()
	{
		if (_native_pointer != 0) {
			JNI.cleanUp(_native_pointer);
			_native_pointer = 0;
		}
	}

	/**
	 * Get converted channel value. 
	 *
	 * {@link #readChannel(TenkiDeviceChannel)} must be called at least once prior to using this method.
	 *
	 * @param chn The channel
	 * @return The converted channel value.
	 **/
	public float getConvertedChannelValue(TenkiDeviceChannel chn)
	{
		return JNI.getConvertedChannelValue(_native_pointer, chn.getId());
	}
	
	/**
	 * Get converted channel unit.
	 *
	 * readChannel must be called at least once prior to using this method.
	 *
	 * @see com.raphnet.tenki.TenkiConstants
	 * @param chn The channel
	 * @return The converted channel unit.
	 **/
	public int getConvertedChannelUnit(TenkiDeviceChannel chn)
	{
		return JNI.getConvertedChannelUnit(_native_pointer, chn.getId());
	}

	/**
	 * Read the specific channel from the device and store the result in memory.
	 *
	 * @throws TenkiException when the read fails.
	 */
	public void readChannel(TenkiDeviceChannel chn) throws TenkiException
	{
		JNI.readChannel(_native_pointer, chn.getId());
	}


}

