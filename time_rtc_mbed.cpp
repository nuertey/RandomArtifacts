https://forums.mbed.com/t/time-null-of-rtc-example-not-synchronizing-with-rtc-when-using-battery/22248/19

Arm Mbed OS support forum
time(NULL) of RTC example not synchronizing with RTC when using battery
Mbed OS
mbed_os

Log In
​
​
time(NULL) of RTC example not synchronizing with RTC when using battery
Mbed OS
mbed_os

395
views

3
links



read
4 min
Jan 2024
Jan 2024

sunnydaywest
Sam Lefebvre

1
Jan 2024
When I create the following code example, the time value is reset to 12:00:00 AM after each power down or hardware reset:

int main()
{
    //set_time(1256729737);
 
    while(1) {
        time_t seconds = time(NULL);
        char buffer[32];
        strftime(buffer, 32, "%I:%M:%S %p", localtime(&seconds));
        printf("%s                  \r", buffer);
        int i = 50000;
        while(i--); // keep busy (sleep gives programming issues and printf cannot flush)
    }
}
When I create the follwowing code the time is memorized after power down or hardware reset (as expected):

int main()
{
    rtc_init();
    //rtc_write(1256729737);

    while(1) {
        time_t seconds = rtc_read();
        char buffer[32];
        strftime(buffer, 32, "%I:%M:%S %p", localtime(&seconds));
        printf("%s                  \r", buffer);
        int i = 50000;
        while(i--); // keep busy (sleep gives programming issues and printf cannot flush)
    }
}
What’s the difference?

Why the example on the RTC reference page is not containing the right info:
https://os.mbed.com/docs/mbed-os/v6.16/apis/time-apis.html

sunnydaywest
Sam Lefebvre

2
Jan 2024
There is another implementation possible:

RealTimeClock rtc;

int main()
{
    rtc.init();
    while(1) {
        time_t seconds =  RealTimeClock::to_time_t(rtc.now());
        char buffer[32];
        strftime(buffer, 32, "%I:%M:%S %p", localtime(&seconds));
        printf("%s                  \r", buffer);
        int i = 50000;
        while(i--); // keep busy (sleep gives programming issues and printf cannot flush)
    }
}

JohnnyK
Jan Kamidra
Jan 2024
Hello,

it is possible, Mbed has a mess in some docs and someting is not explained to details.

However, did you tried it with the set_time() function? You have comented out this one. It looks like this is the magic.

github.com
ARMmbed/mbed-os/blob/baf6a3022a328b91713e03fd88f65126a9a53f01/platform/source/mbed_rtc_time.cpp#L134-L138
void set_time(time_t t)
{
    const struct timeval tv = { t, 0 };
    settimeofday(&tv, NULL);
}
github.com
ARMmbed/mbed-os/blob/baf6a3022a328b91713e03fd88f65126a9a53f01/platform/source/mbed_rtc_time.cpp#L81-L93
int settimeofday(const struct timeval *tv, MBED_UNUSED const struct timezone *tz)
{
    _mutex->lock();
    if (_rtc_init != NULL) {
        _rtc_init();
    }
    if (_rtc_write != NULL) {
        _rtc_write(tv->tv_sec);
    }
    _mutex->unlock();
    return 0;
}
Also this is not exact i think

while(i--); // keep busy (sleep gives programming issues and printf cannot flush)

Just try to add new line symbol \n to

        printf("%s                  \r\n", buffer);
BR, Jan



sunnydaywest
Sam Lefebvre

3
Jan 2024
With set_time() you can set it (and it’s using RTC because you can test it by shortening your crystal with a pincet to see that the clock stops) but you may call it only once otherwise it’s logical that the clock resets after each reboot. That’s the reason that I put it in comment. I call it once, put in command, recompile and program again. But in that case time() always starts at twelve o clock. It is not using the RTC value anymore.

I also see that the RTC Init function is never called automatically.

I was searching on the internet how they make the couping between the RTC and the system time(). It should work as you call the RTC api, but it isn’t. I remember in embedded Linux systems that there is something as a separate HW and SW clock.

Remember the ticket about the hiccups. If I want to use a STLinkV2 clone, the HW reset pin is not working and programming during sleep is not possible. It works with the original programmer because it uses HW reset.

I use \r because I want to see the clock overwritten in the console.



sunnydaywest
Sam Lefebvre

2
Jan 2024
I found it. In the first case you have to do RTC init manually. Nobody knows why this is not already in mbed.

This is working with battery:

int main()
{
    rtc_init();
    //set_time(1256729737);

    while(1) {
        time_t seconds = time(NULL);
        char buffer[32];
        strftime(buffer, 32, "%I:%M:%S %p", localtime(&seconds));
        printf("%s\r", buffer);
        int i = 100000;
        while(i--); // keep busy (sleep gives programming issues and printf cannot flush)
    }
}


sunnydaywest
Sam Lefebvre

3

sunnydaywest
Jan 2024
In mbed_rtc_time.cpp there is a definition that attaches the RTC functions to the C time functions.

#if DEVICE_RTC

static void (*_rtc_init)(void) = rtc_init;
static int (*_rtc_isenabled)(void) = rtc_isenabled;
static time_t (*_rtc_read)(void) = rtc_read;
static void (*_rtc_write)(time_t t) = rtc_write;
There is a function to overwrite it which is only meant or unit testing purposes or if you want to use a different clock source.

The C-function time is defined as:

time_t time(time_t *timer)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    if (timer != NULL) {
        *timer = tv.tv_sec;
    }

    return tv.tv_sec;
}
When doing settime, the init is done:

void set_time(time_t t)
{
    const struct timeval tv = { t, 0 };
    settimeofday(&tv, NULL);
}

int settimeofday(const struct timeval *tv, MBED_UNUSED const struct timezone *tz)
{
    _mutex->lock();
    if (_rtc_init != NULL) {
        _rtc_init();
    }
    if (_rtc_write != NULL) {
        _rtc_write(tv->tv_sec);
    }
    _mutex->unlock();

    return 0;
}
But this is contradictory, you want to have init also if you won’t call set_time in order to have the RTC value runned on battery.

The function gettimeofday is surprisingly resetting the clock when it’s not enabled instead of only initializing it.

int gettimeofday(struct timeval *tv, MBED_UNUSED void *tz)
{
    _mutex->lock();
    if (_rtc_isenabled != NULL) {
        if (!(_rtc_isenabled())) {
            set_time(0);
        }
    }

    time_t t = (time_t) - 1;
    if (_rtc_read != NULL) {
        t = _rtc_read();
    }

    tv->tv_sec  = t;
    tv->tv_usec = 0;

    _mutex->unlock();

    return 0;
}


sunnydaywest
Sam Lefebvre
Jan 2024
In TARGET_STM mbed_overrides.c there is a function mbed_sdk_init() that contains RTC init between

#if DEVICE_RTC 
...
#endif /* DEVICE_RTC */
Which is supposed to be called.

In rtc_api.c there is a function rtc_init() that is doing less or more the same.
I think something is wrong in the mbed_sdk_init(), because it only works only after calling rtc_init().



sunnydaywest
Sam Lefebvre

1
Jan 2024
If I put this part in the RTC config it’s solved. I will create a ticket in mbed project.

#if defined __HAL_RCC_RTCAPB_CLK_ENABLE
    __HAL_RCC_RTCAPB_CLK_ENABLE();
#endif /* __HAL_RCC_RTCAPB_CLK_ENABLE */
github.com/ARMmbed/mbed-os
time(NULL) not returning real RTC value after reboot
opened  Jan 16, 2024
lefebvresam lefebvresam
priority: untriaged component: untriaged
### Description of defect

The following code starts counting at 12:00:00 AM a…



MultipleMonomials
Jamie Smith

1
Jan 2024
Wow, this is silly, the same LSE clock init code is duplicated between mbed_overrides.c, rtc_api.c, and lp_ticker.c for STM. Whichever of those gets executed first is the one which actually configures the LSE oscillator. Those should really be combined.

Update: Turns out that the code is not completely duplicated between those three places, not really. The RTC clock init code is duplicated between mbed_overrides and rtc_api, but the overrides version does not do LSE init for some reason?

Regarding your issue, I think this is a legitimate case of undefined behavior in Mbed. I had actually assumed, and written in wiki pages, that the RTC is not activated until user code calls rtc_init(). So, the example you provided in your first post would be correct behavior.

However, the Mbed example code implies different behavior, and also it seems like STM32 got confused and basically set up the RTC clock to always initialize on boot.

Thinking about this from a little further back, I think the biggest source of weirdness is that, if gettimeofday() is called and the RTC is not initialized, it sets the time to 0 instead of just initializing the RTC. It would be much more logical for it to just call rtc_init() and use whatever time value the RTC is now providing.

So, I think that to fix this, we should do the following:

Update gettimeofday() to call rtc_init() if the RTC is not enabled
Update TARGET_STM/mbed_overrides.c to not initialize the RTC clock
Update rtc_init() and mbed_overrides.c to remove duplicated code


MultipleMonomials
Jamie Smith
Jan 2024
And regarding why your fix actually works… I’m a bit confused. Because ultimately, calling time() before rtc_init() should always result in the RTC time starting from 0 at boot, because the mbed_rtc_time.cpp code is doing rtc_write(0). Are you sure that adding that clock enable line is causing it to not start from 0 at boot?



MultipleMonomials
Jamie Smith
Jan 2024
Took a crack at fixing this in this PR: [draft] Don't reset the RTC time from gettimeofday(), plus other RTC cleanups by multiplemonomials · Pull Request #210 · mbed-ce/mbed-os · GitHub.

It would be great if you could test it out!



sunnydaywest
Sam Lefebvre
Jan 2024
Why you would not do the initialization automatically? It seems to be more logical that you initialize as soon as the RTC is used and available to that you always get the right time value if you use time(NULL).



MultipleMonomials
Jamie Smith
Jan 2024
We don’t want the RTC to auto init at boot because that starts the 32kHz oscillator on some targets, and people might not want the 32kHz oscillator to start when it’s not used in code.

With the change in my PR, the RTC doesn’t start at boot, but it does start the first time you get the time (e.g. with time()), And it won’t reset to 0 this time with the set_time() call removed. So I claim it should do what you want.


sunnydaywest
Sam Lefebvre
Jan 2024
I think with ‘start’ you mean ‘connected to…’ because it has to stay running when battery is connected and power down.



sunnydaywest
Sam Lefebvre

2

MultipleMonomials
Jan 2024
Regarding to your question it seems that _rtc_isenabled() returns true as soon as __HAL_RCC_RTCAPB_CLK_ENABLE(); is executed. That’s the reason that with my fix in the mbed_sdk_init() the call to set_time(0); is never done anymore.

I will not dive in all the STM32 details but in CubeIDE I have seen that there are 3 inits:

    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();
    __HAL_RCC_RTCAPB_CLK_ENABLE();
    __HAL_RCC_RTCAPB_CLKAM_ENABLE();
I don’t know to what extend the other two are relevvant?

I see this one is in rtc_init() but seems not mandatory to run (or maybe already called somewhere else):
__HAL_RCC_RTC_ENABLE();

And what about
__HAL_RCC_RTCAPB_CLKAM_ENABLE();



sunnydaywest
Sam Lefebvre

sunnydaywest
Jan 2024
I’m now realizing that I was creating my tickets in the wrong repo.

It was in the “dead” original repo like also others are still doing:


GitHub

GitHub - ARMmbed/mbed-os: Arm Mbed OS is a platform operating system designed...
Arm Mbed OS is a platform operating system designed for the internet of things - GitHub - ARMmbed/mbed-os: Arm Mbed OS is a platform operating system designed for the internet of things

That’s very confusing. If you create a new project with mbed-tools configure, the wrong repo is checked out by default. It means that I’m constantly interacting with outdated mbed code and maybe facing already solved issues. And I will not be the first…

I don’t know if it is possible to use the right repo by default to not confuse other users?

It should be:


GitHub

GitHub - mbed-ce/mbed-os: Arm Mbed OS is a platform operating system designed...
Arm Mbed OS is a platform operating system designed for the internet of things - GitHub - mbed-ce/mbed-os: Arm Mbed OS is a platform operating system designed for the internet of things



MultipleMonomials
Jamie Smith
Jan 2024
Oh well that’s because the Mbed CLI stuff is from ARM, so it still uses the official (and mostly dead) repo. Jan and I are developing the Mbed CE fork which has its own build system and infrastructure. You’re welcome to create tickets on the official repo, but the likelihood of anyone doing anything about them at this point is… not great.



sunnydaywest
Sam Lefebvre
Jan 2024
Lol, I already migrated my tickets to the CE and will try the new doc. I will start from a fresh ‘board-bringup’ project and transfer all the knowledge of the previous one to the new one. I will also make a pull request for the added HSE stuff on the MCU_STM32U575xG.

================================================================================

mBed RTC get time hours, mins and seconds
Mbed OS
Best practice
rtc
Feb 2021
Feb 2021

jayaram12391
Jayaram Subramanian
Feb 2021
Hello everyone,

I need to get the current hour, min and sec using RTC. I am using stm32f767ZI. I used the example code for RTC and I am able to see the current time.
Can anyone tell me how to get the hours, min and sec separately.

time_t seconds = time(NULL);
printf(“Time as a basic string = %s\n”, ctime(&seconds));
strftime(buffer, 32, “%H:%M:%S %p\n”, localtime(&seconds));
printf(“Time as a custom formatted string = %s”, buffer);

This code prints out the time but I am not clear how to get the hours, mins and sec as integers separately. Thanks.




1.0k
views

1
link



boraozgen
Bora Özgen
Feb 2021

tutorialspoint.com
C library function - localtime()
C library function - localtime(), The C library function struct tm *localtime(const time_t *timer) uses the time pointed by timer to fill a tm structure with the values that represent the corres



jayaram12391
Jayaram Subramanian
Feb 2021
Thanks. I did according to the example and It worked. This is the code i used.
struct tm *current_time;
current_time = localtime(&seconds);
printf(“Current hour: %d\n”, current_time->tm_hour);


