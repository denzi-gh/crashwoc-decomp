unsigned long OSGetTick(void);

static int QueryPerformanceFrequency(int *frequency)
{
    *frequency = 40500000;
    return 1;
}

static int QueryPerformanceCounter(int *counter)
{
    *counter = OSGetTick();
    return 1;
}

unsigned long timeGetTime(void)
{
    return OSGetTick();
}

double TimerGetFPS(void)
{
    static int frame = 0;
    static int frame_2 = 0;
    static int pcount_f = 0;
    static double nFPS;
    static double aFPS;
    static double MxFPS;
    static double MnFPS;
    static int freqi;
    static int Time3i;
    static double OldTime3;
    static double Time3;
    static double freq;
    static double nowTime;
    static double maxTime = 0.00001;
    static double minTime = 99999.0;
    static double totalTime = 0.0;
    static double aveTime;

    if (frame == 0) {
        pcount_f = QueryPerformanceFrequency(&freqi);
        if (pcount_f) {
            QueryPerformanceCounter(&Time3i);
            freq = 1000000.0 / (double)freqi;
            Time3 = (double)Time3i * freq;
            OldTime3 = Time3;
        } else {
            Time3 = (double)timeGetTime();
        }
    }

    if (pcount_f) {
        QueryPerformanceCounter(&Time3i);
        OldTime3 = Time3;
        Time3 = (double)Time3i * freq;
    } else {
        OldTime3 = Time3;
        Time3 = (double)timeGetTime();
    }

    nowTime = Time3 - OldTime3;
    if (nowTime > maxTime) {
        maxTime = nowTime;
    }
    if (nowTime < minTime) {
        minTime = nowTime;
    }

    totalTime += nowTime;
    aveTime = totalTime / ((double)frame + 1.0);
    aFPS = 1000000.0 / aveTime;
    frame++;
    nFPS = 1000000.0 / nowTime;
    MnFPS = 1000000.0 / maxTime;
    MxFPS = 1000000.0 / minTime;

    if (frame > 1000000) {
        frame = 0;
        totalTime = 0.0;
        frame_2++;
    }

    return nFPS;
}
