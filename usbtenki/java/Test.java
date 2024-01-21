import com.raphnet.tenki.*;

public class Test
{
	public static final void main(String args[])
	{
		USBTenkiDevice demo;

		try {
			System.out.println("Creating USBTEnkiDevice instance");
			demo = new USBTenkiDevice("B10004");

			System.out.println("Opening device");
			demo.open();

			System.out.println("ok!");

			System.out.println("Type: " + demo.getType());
			System.out.println("Serial: " + demo.getSerial());
			System.out.println("Version: " + demo.getVersion());

			TenkiDeviceChannel chns[] = demo.getAvailableChannels();

			for (int i = 0; i<chns.length; i++) {
				/* Skip unused channels */	
				if (chns[i].getChipId()==TenkiConstants.CHIP_NONE)
					continue;

				System.out.println("Channel " + chns[i].getId() + ": " + 
				chns[i].getName() + " [" + chns[i].getTypeName() + "], chip: " + 
				chns[i].getChipId() + " , Value: " + chns[i].getValue() + " " + TenkiConstants.unitToString(chns[i].getUnit()) );
			}

		} catch (Exception e) {
			e.printStackTrace();
		}
	}	
}
