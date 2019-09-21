
#include <ArduinoJson.h>



class CustomPalette : public CRGBPalette16
{
public:
    CustomPalette(const CRGB &c1, const CRGB &c2, const CRGB &c3, const CRGB &c4, const CRGB &c5)
    {
        fivePalletes(&(entries[0]), 16, c1, c2, c3, c4, c5);
    }
    CustomPalette(const CRGB &c1, const CRGB &c2, const CRGB &c3, const CRGB &c4, const CRGB &c5, const CRGB &c6)
    {
        sixPalletes(&(entries[0]), 16, c1, c2, c3, c4, c5, c6);
    }

    void fivePalletes(CRGB *leds, uint16_t numLeds, const CRGB &c1, const CRGB &c2, const CRGB &c3, const CRGB &c4, const CRGB &c5)
    {
        uint16_t onefourth = (numLeds / 4);
        uint16_t half = ((numLeds * 2) / 4);
        uint16_t threefourth = ((numLeds * 3) / 4);
        uint16_t last = numLeds - 1;
        fill_gradient_RGB(leds, 0, c1, onefourth, c2);
        fill_gradient_RGB(leds, onefourth, c2, half, c3);
        fill_gradient_RGB(leds, half, c3, threefourth, c4);
        fill_gradient_RGB(leds, threefourth, c4, last, c5);
    }

    void sixPalletes(CRGB *leds, uint16_t numLeds, const CRGB &c1, const CRGB &c2, const CRGB &c3, const CRGB &c4, const CRGB &c5, const CRGB &c6)
    {
        uint16_t onefifth = (numLeds / 5);
        uint16_t twofifth = ((numLeds * 2) / 5);
        uint16_t threefifths = ((numLeds * 3) / 5);
        uint16_t fourfifths = ((numLeds * 4) / 5);
        uint16_t last = numLeds - 1;
        fill_gradient_RGB(leds, 0, c1, onefifth, c2);
        fill_gradient_RGB(leds, onefifth, c2, twofifth, c3);
        fill_gradient_RGB(leds, twofifth, c3, threefifths, c4);
        fill_gradient_RGB(leds, threefifths, c4, fourfifths, c5);
        fill_gradient_RGB(leds, fourfifths, c5, last, c6);
    }
};



CRGBPalette16 createNewPalette(const JsonArray array)
{
    CRGB t[array.size()];
    for (uint8_t i = 0; i < array.size(); i++)
    {
        t[i] = CRGB(array.getElement(i).as<long>());
    }

    if (array.size() == 1)
    {
        return CRGBPalette16(t[0]);
    }
    else if (array.size() == 2)
    {
        return CRGBPalette16(t[0], t[1]);
    }

    else if (array.size() == 3)
    {
        return CRGBPalette16(t[0], t[1], t[2]);
    }

    else if (array.size() == 4)
    {
        return CRGBPalette16(t[0], t[1], t[2], t[3]);
    }

    else if (array.size() == 5)
    {
        return CustomPalette(t[0], t[1], t[2], t[3], t[4]);
    }

    else
    {
        return CustomPalette(t[0], t[1], t[2], t[3], t[4], t[5]);
    }
}
