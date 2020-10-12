using System;
using System.Collections.Generic;
using System.Text;

namespace MCCTASGUI.Plugins
{
    enum RandomizerMode
    {
        Global,     // Use the same configuration on every level
        Per_Level,  // Use a static configuration for each level
        Rotating,   // Change configuration on a timer
    }

    class RandomizerConfig
    {
        RandomizerMode Mode = RandomizerMode.Global;
        float SecondsBetweenRotations = 30.0f;
        int ChangesPerRotation = 1;

        List<string> AvailableCommands;

        public static readonly string[] DefaultCommands =
        {
            "",
            "",
            ""
        };

        void Import(string ImportString)
        {

        }
        string Export()
        {
            string Out = "";

            return Out;
        }
    }

    class Randomizer
    {
        RandomizerConfig Config;

        public Randomizer()
        {

        }


    }
}
