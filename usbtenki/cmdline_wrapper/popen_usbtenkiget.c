#include <stdio.h>


int getTempAndHumidity(float *temp, float *hum)
{
	FILE *fptr;
	float t, h;
	int n, res;

	fptr = popen("usbtenkiget -i a", "r");
	if (!fptr) {
		perror("popen");
		return -1;
	}

	n = fscanf(fptr, "%f, %f", &t, &h);
	res = pclose(fptr);

	if (res==-1)
		return -2;

	if (n!=2) {
		return -3;
	}
	
	if (temp)
		*temp = t;
	if (hum)
		*hum = h;
	
	return 0;
}

int main(void)
{
	float temperature, humidity;
	int res;

	res = getTempAndHumidity(&temperature, &humidity);
	if (res<0) {
		return res;
	}

	printf("Temperature: %.2f\nHumidity: %.2f\n", temperature, humidity);
	
	return 0;
}
