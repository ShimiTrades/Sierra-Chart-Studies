// Shimi Swingarms with Risk - Sierra Chart Conversion

#include "sierrachart.h"

SCDLLName("Shimi Swingarms with Risk 6")

SCSFExport scsf_ShimiSwingarms6(SCStudyInterfaceRef sc)
{
    SCSubgraphRef TrailingStop = sc.Subgraph[0];
    SCSubgraphRef Extremum = sc.Subgraph[1];  // Not displayed
    SCSubgraphRef Fib1 = sc.Subgraph[2];
    SCSubgraphRef Fib2 = sc.Subgraph[3];
    SCSubgraphRef Fib3 = sc.Subgraph[4];

    SCInputRef ATRPeriod = sc.Input[0];
    SCInputRef ATRFactor = sc.Input[1];
    SCInputRef FirstTrade = sc.Input[2];
    SCInputRef TrailType = sc.Input[3];
    SCInputRef Fib1Level = sc.Input[4];
    SCInputRef Fib2Level = sc.Input[5];
    SCInputRef Fib3Level = sc.Input[6];

    if (sc.SetDefaults)
    {
        sc.GraphName = "Shimi Swingarms with Risk";
        sc.StudyDescription = "Trailing Stop System with ATR-Based Risk Management & Fibonacci Levels.";
		
		// Set the study to show in Chart Region 1 (main price chart)
		sc.GraphRegion = 0;

        ATRPeriod.Name = "ATR Period";
        ATRPeriod.SetInt(28);

        ATRFactor.Name = "ATR Factor";
        ATRFactor.SetFloat(5.0);

        FirstTrade.Name = "First Trade";
        FirstTrade.SetInt(1); // 1 = Long, 0 = Short

        TrailType.Name = "Trail Type";
        TrailType.SetInt(0); // 0 = Modified, 1 = Unmodified

        Fib1Level.Name = "Fib 1 Level";
        Fib1Level.SetFloat(61.8);

        Fib2Level.Name = "Fib 2 Level";
        Fib2Level.SetFloat(78.6);

        Fib3Level.Name = "Fib 3 Level";
        Fib3Level.SetFloat(88.6);

        TrailingStop.Name = "Trailing Stop";
        TrailingStop.PrimaryColor = RGB(128, 0, 255);
        TrailingStop.DrawStyle = DRAWSTYLE_LINE;

        Extremum.Name = "Extremum";
        Extremum.PrimaryColor = RGB(255, 0, 0);
        Extremum.DrawStyle = DRAWSTYLE_IGNORE;

        Fib1.Name = "Fib 1";
        Fib1.PrimaryColor = RGB(98, 98, 98);
        Fib1.DrawStyle = DRAWSTYLE_LINE;

        Fib2.Name = "Fib 2";
        Fib2.PrimaryColor = RGB(98, 98, 98);
        Fib2.DrawStyle = DRAWSTYLE_LINE;

        Fib3.Name = "Fib 3";
        Fib3.PrimaryColor = RGB(98, 98, 98);
        Fib3.DrawStyle = DRAWSTYLE_LINE;

        sc.AutoLoop = 1;
        return;
    }

    // ✅ Only process bar after it has closed
   if (sc.GetBarHasClosedStatus() == BHCS_BAR_HAS_NOT_CLOSED)
{
    // Maintain last value so label doesn't disappear during live bar
    TrailingStop[sc.Index] = TrailingStop[sc.Index - 1];
    Fib1[sc.Index] = Fib1[sc.Index - 1];
    Fib2[sc.Index] = Fib2[sc.Index - 1];
    Fib3[sc.Index] = Fib3[sc.Index - 1];
    Extremum[sc.Index] = Extremum[sc.Index - 1];

    return;
}

    // Calculate True Range manually
    float HiLo = min(sc.High[sc.Index] - sc.Low[sc.Index], 1.5f * sc.BaseDataIn[SC_HIGH][sc.Index]);
    float HRef = (sc.Low[sc.Index] <= sc.High[sc.Index - 1]) ? (sc.High[sc.Index] - sc.Close[sc.Index - 1])
                : (sc.High[sc.Index] - sc.Close[sc.Index - 1]) - 0.5f * (sc.Low[sc.Index] - sc.High[sc.Index - 1]);
    float LRef = (sc.High[sc.Index] >= sc.Low[sc.Index - 1]) ? (sc.Close[sc.Index - 1] - sc.Low[sc.Index])
                : (sc.Close[sc.Index - 1] - sc.Low[sc.Index]) - 0.5f * (sc.Low[sc.Index - 1] - sc.High[sc.Index]);

    float TrueRange = (TrailType.GetInt() == 0) ? max(HiLo, max(HRef, LRef))
                      : max(sc.High[sc.Index] - sc.Low[sc.Index],
                            max(fabs(sc.High[sc.Index] - sc.Close[sc.Index - 1]),
                                fabs(sc.Low[sc.Index] - sc.Close[sc.Index - 1])));

    // ATR Calculation using Wilder’s Moving Average
    SCFloatArrayRef TrueRangeArray = sc.Subgraph[5];
    SCFloatArrayRef ATRArray = sc.Subgraph[6];

    TrueRangeArray[sc.Index] = TrueRange;
    sc.MovingAverage(TrueRangeArray, ATRArray, MOVAVGTYPE_WILDERS, ATRPeriod.GetInt());
    float ATR = ATRArray[sc.Index];

    float Loss = ATRFactor.GetFloat() * ATR;

    // Restore static state variables
    static int state = 0; // 0 = Init, 1 = Long, 2 = Short
    static float trail = 0;
    static float ex = 0;

    if (sc.Index == 0)
    {
        state = (FirstTrade.GetInt() == 1) ? 1 : 2;
        trail = (state == 1) ? sc.Close[sc.Index] - Loss : sc.Close[sc.Index] + Loss;
        ex = sc.Close[sc.Index];
    }
    else
    {
        if (state == 1) // Long
        {
            if (sc.Close[sc.Index] < trail)
            {
                state = 2;
                trail = sc.Close[sc.Index] + Loss;
                ex = sc.Low[sc.Index];
            }
            else
            {
                trail = max(trail, sc.Close[sc.Index] - Loss);
                ex = max(ex, sc.High[sc.Index]);
            }
        }
        else if (state == 2) // Short
        {
            if (sc.Close[sc.Index] > trail)
            {
                state = 1;
                trail = sc.Close[sc.Index] - Loss;
                ex = sc.High[sc.Index];
            }
            else
            {
                trail = min(trail, sc.Close[sc.Index] + Loss);
                ex = min(ex, sc.Low[sc.Index]);
            }
        }
    }

    // Fibonacci levels (computed after extremum + trail updates)
    float Fib1Value = ex + (trail - ex) * (Fib1Level.GetFloat() / 100.0f);
    float Fib2Value = ex + (trail - ex) * (Fib2Level.GetFloat() / 100.0f);
    float Fib3Value = ex + (trail - ex) * (Fib3Level.GetFloat() / 100.0f);

    // Assign output values
    TrailingStop[sc.Index] = trail;
    Extremum[sc.Index] = ex;
    Fib1[sc.Index] = Fib1Value;
    Fib2[sc.Index] = Fib2Value;
    Fib3[sc.Index] = Fib3Value;
}
