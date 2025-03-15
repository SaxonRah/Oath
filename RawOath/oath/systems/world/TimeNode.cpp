#include <TimeNode.hpp>

TimeNode::TimeNode(const std::string& name)
    : TANode(name)
    , day(1)
    , hour(6)
    , season("spring")
    , timeOfDay("morning")
{
}

void TimeNode::advanceHour(GameContext* context)
{
    hour++;

    if (hour >= 24) {
        hour = 0;
        day++;

        // Update season every 90 days
        if (day % 90 == 0) {
            if (season == "spring")
                season = "summer";
            else if (season == "summer")
                season = "autumn";
            else if (season == "autumn")
                season = "winter";
            else if (season == "winter")
                season = "spring";
        }

        if (context) {
            context->worldState.advanceDay();
        }
    }

    // Update time of day
    if (hour >= 5 && hour < 12)
        timeOfDay = "morning";
    else if (hour >= 12 && hour < 17)
        timeOfDay = "afternoon";
    else if (hour >= 17 && hour < 21)
        timeOfDay = "evening";
    else
        timeOfDay = "night";

    std::cout << "Time: Day " << day << ", " << hour << ":00, " << timeOfDay
              << " (" << season << ")" << std::endl;
}

std::vector<TAAction> TimeNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    actions.push_back({ "wait_1_hour", "Wait 1 hour", [this]() -> TAInput {
                           return {
                               "time_action",
                               { { "action", std::string("wait") }, { "hours", 1 } }
                           };
                       } });

    actions.push_back(
        { "wait_until_morning", "Wait until morning", [this]() -> TAInput {
             return { "time_action",
                 { { "action", std::string("wait_until") },
                     { "time", std::string("morning") } } };
         } });

    return actions;
}

bool TimeNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "time_action") {
        std::string action = std::get<std::string>(input.parameters.at("action"));

        if (action == "wait") {
            int hours = std::get<int>(input.parameters.at("hours"));
            // In a real game, this would trigger events, status changes, etc.
            std::cout << "Waiting for " << hours << " hours..." << std::endl;

            // Stay in same node after waiting
            outNextNode = this;
            return true;
        } else if (action == "wait_until") {
            std::string targetTime = std::get<std::string>(input.parameters.at("time"));
            // Calculate hours to wait
            int hoursToWait = 0;

            if (targetTime == "morning" && timeOfDay != "morning") {
                if (hour < 5)
                    hoursToWait = 5 - hour;
                else
                    hoursToWait = 24 - (hour - 5);
            }
            // Add other time of day calculations

            std::cout << "Waiting until " << targetTime << " (" << hoursToWait
                      << " hours)..." << std::endl;

            // Stay in same node after waiting
            outNextNode = this;
            return true;
        }
    }

    return TANode::evaluateTransition(input, outNextNode);
}
