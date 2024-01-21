package com.raphnet.tenki;

/**
 * Data source interface for TenkiDeviceChannels.
 *
 * TenkiDeviceChannel uses classes implementing this interface as
 * a way to obtain data and information about the channel they are 
 * representing.
 */
public interface TenkiChannelDataSource
{
	/**
	 * Communicate with the device and fetch the channel data.
	 *
	 * This method shall read the value from the device. The resulting
	 * value and unit shall be kept to be fetched by getConvertedChannelValue for
	 * instance.
	 */
	public void readChannel(TenkiDeviceChannel chn) throws TenkiException;

	/**
	 * Fetch the value of a previously read channel.
	 */
	public float getConvertedChannelValue(TenkiDeviceChannel chn);
	
	/**
	 * Fetch the unit of a previously read channel.
	 * @return One of the UNIT_xxx constants from TenkiConstants
	 */
	public int getConvertedChannelUnit(TenkiDeviceChannel chn);
}

