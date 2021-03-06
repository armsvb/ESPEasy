#ifdef USES_P066
//#######################################################################################################
//#################################### Plugin 066: VEML6040 RGBW ##############################
//#######################################################################################################

// ESPEasy Plugin to scan color values (RGBW) with chip VEML6040
// written by Jochen Krapf (jk@nerd2nerd.org)

// Datasheet: https://www.vishay.com/docs/84276/veml6040.pdf
// Application Note: www.vishay.com/doc?84331


#ifdef PLUGIN_BUILD_TESTING

#define PLUGIN_066
#define PLUGIN_ID_066         66
#define PLUGIN_NAME_066       "Color - VEML6040 [TESTING]"
#define PLUGIN_VALUENAME1_066 "R"
#define PLUGIN_VALUENAME2_066 "G"
#define PLUGIN_VALUENAME3_066 "B"
#define PLUGIN_VALUENAME4_066 "W"

#define VEML6040_ADDR 0x10

// #include <*.h>   // no include needed
#include <math.h>   // no include needed

#ifndef CONFIG
#define CONFIG(n) (Settings.TaskDevicePluginConfig[event->TaskIndex][n])
#endif


boolean Plugin_066(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_066;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].VType = SENSOR_TYPE_QUAD;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 4;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].TimerOptional = false;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_066);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_066));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_066));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_066));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_066));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        int optionValues[1] = { VEML6040_ADDR };
        addFormSelectorI2C(F("i2c_addr"), 1, optionValues, VEML6040_ADDR);   //Only for display I2C address

        String optionsMode[6] = { F("40ms (16496)"), F("80ms (8248)"), F("160ms (4124)"), F("320ms (2062)"), F("640ms (1031)"), F("1280ms (515)") };
        addFormSelector(F("Integration Time (Max Lux)"), F("itime"), 6, optionsMode, NULL, CONFIG(1));

        String optionsVarMap[6] = {
          F("R, G, B, W"),
          F("r, g, b, W - relative rgb [&#37;]"),
          F("r, g, b, W - relative rgb^Gamma [&#37;]"),
          F("R, G, B, Color Temperature [K]"),
          F("R, G, B, Ambient Light [Lux]"),
          F("Color Temperature [K], Ambient Light [Lux], Y, W") };
        addFormSelector(F("Value Mapping"), F("map"), 6, optionsVarMap, NULL, CONFIG(2));

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        //CONFIG(0) = getFormItemInt(F("i2c_addr"));
        CONFIG(1) = getFormItemInt(F("itime"));
        CONFIG(2) = getFormItemInt(F("map"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        VEML6040_Init(CONFIG(1));

        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        float R, G, B, W;

        R = VEML6040_GetValue(0x08);
        G = VEML6040_GetValue(0x09);
        B = VEML6040_GetValue(0x0A);
        W = VEML6040_GetValue(0x0B);

        switch (CONFIG(2))
        {
          default:
          case 0:
          {
            UserVar[event->BaseVarIndex + 0] = R;
            UserVar[event->BaseVarIndex + 1] = G;
            UserVar[event->BaseVarIndex + 2] = B;
            UserVar[event->BaseVarIndex + 3] = W;
            break;
          }
          case 1:
          {
            UserVar[event->BaseVarIndex + 0] = Plugin_066_CalcRelW(R, W) * 100.0;
            UserVar[event->BaseVarIndex + 1] = Plugin_066_CalcRelW(G, W) * 100.0;
            UserVar[event->BaseVarIndex + 2] = Plugin_066_CalcRelW(B, W) * 100.0;
            UserVar[event->BaseVarIndex + 3] = W;
            break;
          }
          case 2:
          {
            UserVar[event->BaseVarIndex + 0] = pow(Plugin_066_CalcRelW(R, W), 0.4545) * 100.0;
            UserVar[event->BaseVarIndex + 1] = pow(Plugin_066_CalcRelW(G, W), 0.4545) * 100.0;
            UserVar[event->BaseVarIndex + 2] = pow(Plugin_066_CalcRelW(B, W), 0.4545) * 100.0;
            UserVar[event->BaseVarIndex + 3] = W;
            break;
          }
          case 3:
          {
            UserVar[event->BaseVarIndex + 0] = R;
            UserVar[event->BaseVarIndex + 1] = G;
            UserVar[event->BaseVarIndex + 2] = B;
            UserVar[event->BaseVarIndex + 3] = Plugin_066_CalcCCT(R, G, B);
            break;
          }
          case 4:
          {
            UserVar[event->BaseVarIndex + 0] = R;
            UserVar[event->BaseVarIndex + 1] = G;
            UserVar[event->BaseVarIndex + 2] = B;
            UserVar[event->BaseVarIndex + 3] = Plugin_066_CalcAmbientLight(G, CONFIG(1));
            break;
          }
          case 5:
          {
            UserVar[event->BaseVarIndex + 0] = Plugin_066_CalcCCT(R, G, B);
            UserVar[event->BaseVarIndex + 1] = Plugin_066_CalcAmbientLight(G, CONFIG(1));
            UserVar[event->BaseVarIndex + 2] = (R + G + B) / 3.0;   //0.299*R + 0.587*G + 0.114*B;
            UserVar[event->BaseVarIndex + 3] = W;
            break;
          }
        }
        success = true;
        break;
      }

  }
  return success;
}


// VEML6040 /////////////////////////////////////////////////////////////

void VEML6040_setControlReg(byte data)
{
	Wire.beginTransmission(VEML6040_ADDR);
  Wire.write(0);   //command 0=control register
	Wire.write(data);   //lsb
  Wire.write(0);   //msb
	Wire.endTransmission();
}

float VEML6040_GetValue(byte reg)
{
	Wire.beginTransmission(VEML6040_ADDR);
	Wire.write(reg);
	Wire.endTransmission(false);
	Wire.requestFrom((uint8_t)VEML6040_ADDR, (uint8_t)0x2);
	if (Wire.available() == 2)
	{
    uint16_t lsb = Wire.read();
    uint16_t msb = Wire.read();
		return (float)((msb << 8) | lsb);
	}
	return -1.0;
}

void VEML6040_Init(byte it)
{
  VEML6040_setControlReg(it << 4);   // IT=it, TRIG=0, AF=0, SD=0
}


float Plugin_066_CalcCCT(float R, float G, float B)
{
  if (G == 0)
    return 0;

  float CCTi = (R - B) / G + 0.5;
  float CCT = 4278.6 * pow(CCTi, -1.2455);
  return CCT;
}

float Plugin_066_CalcAmbientLight(float G, byte it)
{
  float Sensitivity[6] = { 0.25168, 0.12584, 0.06292, 0.03146, 0.01573, 0.007865 };

  return G * Sensitivity[it];
}

float Plugin_066_CalcRelW(float X, float W)
{
  if (W == 0)
    return 0;

  return X / W;
}

#endif
#endif // USES_P066
