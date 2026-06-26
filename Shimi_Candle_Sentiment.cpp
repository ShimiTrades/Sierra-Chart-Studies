#include "sierrachart.h"

SCDLLName("Shimi Sentiment Candle HollowFill Final")

SCSFExport scsf_ShimiSentimentCandleColoringFinal(SCStudyInterfaceRef sc)
{
    if (sc.SetDefaults)
    {
        sc.GraphName = "Shimi Sentiment Candle Coloring (Final)";
        sc.StudyDescription = "Applies fill and outline based on persistent sentiment logic.";
        sc.AutoLoop = 1;
        sc.GraphRegion = 0;

        sc.Input[0].Name = "Sentiment Threshold (0–1)";
        sc.Input[0].SetFloat(0.75f);

        sc.Input[1].Name = "Bullish Color";
        sc.Input[1].SetColor(RGB(91, 221, 227)); // Cyan

        sc.Input[2].Name = "Bearish Color";
        sc.Input[2].SetColor(RGB(225, 147, 0));  // Orange

        // Subgraph 0: Candle outline
        sc.Subgraph[0].Name = "Outline";
        sc.Subgraph[0].DrawStyle = DRAWSTYLE_COLOR_BAR_HOLLOW;
        sc.Subgraph[0].PrimaryColor = COLOR_WHITE;

        // Subgraph 1: Candle fill
        sc.Subgraph[1].Name = "Fill";
        sc.Subgraph[1].DrawStyle = DRAWSTYLE_COLOR_BAR_CANDLE_FILL;
        sc.Subgraph[1].PrimaryColor = COLOR_WHITE;

        // Subgraph 2: Sentiment (used only for internal tracking)
        sc.Subgraph[2].Name = "Sentiment Value";
        sc.Subgraph[2].DrawStyle = DRAWSTYLE_IGNORE;

        return;
    }

    int i = sc.Index;
    float threshold = sc.Input[0].GetFloat();
    float invThreshold = 1.0f - threshold;
    int BullColor = sc.Input[1].GetColor();
    int BearColor = sc.Input[2].GetColor();

    // Candle metrics
    float range = sc.High[i] - sc.Low[i];
    float pct = (range > 0) ? (sc.Close[i] - sc.Low[i]) / range : 0.0f;
    bool closeDown = sc.Close[i] < sc.Open[i];

    // Determine sentiment state
    float sentiment = (i == 0) ? 0 : sc.Subgraph[2][i - 1]; // default to previous bar

    if (sc.Close[i] > sc.High[i - 1] && pct >= threshold)
        sentiment = 1; // bullish
    else if (sc.Close[i] < sc.Low[i - 1] && pct <= invThreshold)
        sentiment = -1; // bearish

    // Save state
    sc.Subgraph[2][i] = sentiment;

    // Outline (always present)
    sc.Subgraph[0][i] = 1;
    sc.Subgraph[0].DataColor[i] =
        (sentiment == 1) ? BullColor :
        (sentiment == -1) ? BearColor :
        RGB(128, 128, 128); // gray for neutral

    // Fill (only for down candles)
    if (closeDown)
    {
        sc.Subgraph[1][i] = 1;
        sc.Subgraph[1].DataColor[i] =
            (sentiment == 1) ? BullColor :
            (sentiment == -1) ? BearColor :
            RGB(128, 128, 128);
    }
    else
    {
        sc.Subgraph[1][i] = 0; // no fill for up candle
    }
}
