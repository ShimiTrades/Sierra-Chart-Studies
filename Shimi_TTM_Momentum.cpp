// TTM Squeeze, Momentum Histogram Only
// Matches the TOS implementation exactly
// by Shimi

#include "sierrachart.h"

SCDLLName("Shimi TTM Squeeze Momentum Histogram Only")

SCSFExport scsf_ShimiTTMMomentumHistogram(SCStudyInterfaceRef sc)
{
    SCInputRef Input_Length = sc.Input[0];
    SCSubgraphRef Momo = sc.Subgraph[0];

    if (sc.SetDefaults)
    {
        sc.GraphName = "TTM Squeeze Momentum Histogram Only";
        sc.StudyDescription = "Exact replication of TOS TTM Squeeze momentum histogram calculation (without squeeze dots).";
        sc.AutoLoop = 1;

        Input_Length.Name = "Length";
        Input_Length.SetInt(20);
        Input_Length.SetIntLimits(1, 200);

        Momo.Name = "Momentum Histogram";
        Momo.DrawStyle = DRAWSTYLE_BAR;
        Momo.LineWidth = 3;
        Momo.PrimaryColor = RGB(0, 255, 255); // Cyan (default upward)
        Momo.DrawZeros = true;

        return;
    }

    int len = Input_Length.GetInt();
    int index = sc.Index;

    // Buffers
    SCFloatArrayRef EMA = sc.Subgraph[1].Data;
    SCFloatArrayRef InertiaInput = sc.Subgraph[2].Data;
    SCFloatArrayRef InertiaOutput = sc.Subgraph[3].Data;

    // EMA of close
    sc.ExponentialMovAvg(sc.Close, EMA, index, len);

    // Calculate K = ((HH + LL) / 2) + EMA
    float HH = sc.GetHighest(sc.High, len);
    float LL = sc.GetLowest(sc.Low, len);
    float K = ((HH + LL) / 2.0f) + EMA[index];

    // Inertia input = close - (K / 2)
    InertiaInput[index] = sc.Close[index] - (K / 2.0f);

    // Apply linear regression (Inertia) to mimic TOS Inertia()
    sc.LinearRegressionIndicator(InertiaInput, InertiaOutput, len);

    float momo = InertiaOutput[index];
    Momo[index] = momo;

    // Color matching TOS logic
    float prev = InertiaOutput[index - 1];
    if (momo > prev && momo > 0)
        Momo.DataColor[index] = RGB(0, 255, 255); // Cyan
    else if (momo < prev && momo > 0)
        Momo.DataColor[index] = RGB(0, 0, 255);   // Blue
    else if (momo < prev && momo < 0)
        Momo.DataColor[index] = RGB(255, 0, 0);   // Red
    else
        Momo.DataColor[index] = RGB(255, 255, 0); // Yellow
}
