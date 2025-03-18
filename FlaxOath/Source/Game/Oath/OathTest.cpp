#include "OathTest.h"
#include "Engine/Core/Log.h"

OathTest::OathTest(const SpawnParams& params)
    : Script(params)
{
    // Enable ticking OnUpdate function
    _tickUpdate = true;
}

void OathTest::OnEnable()
{
    // Here you can add code that needs to be called when script is enabled (eg. register for events)
}

void OathTest::OnDisable()
{
    // Here you can add code that needs to be called when script is disabled (eg. unregister from events)
}

void OathTest::OnUpdate()
{
    // Here you can add code that needs to be called every frame
    LOG(Info, "Hello from C++!");
}
