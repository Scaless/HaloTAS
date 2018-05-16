using System;
using System.Windows.Forms;
using System.Collections.Generic;
using System.Threading;

namespace TASInterface
{
    public partial class frmTAS : Form
    {
        int ADDR_INPUT_TICK = 0x006F1D8C;
        int ADDR_KEYBOARD_INPUT = 0x006B1620;
        int ADDR_FRAMES_SINCE_LEVEL_START = 0x00746F88;
        int ADDR_WINDOW_FOCUS = 0x00721E8C;
        int ADDR_MAP_STRING = 0x006A8174;
        int ADDR_SUBLEVEL_INDICATOR = 0x00746F90;
        int ADDR_UPDOWNVIEW = 0x402AD4BC;
        int ADDR_LEFTRIGHTVIEW = 0x402AD4B8;
        int ADDR_LEFTMOUSE = 0x006B1818;
        int ADDR_RIGHTMOUSE = 0x006B181A;

        public frmTAS()
        {
            InitializeComponent();
        }

        private void frmTAS_Load(object sender, EventArgs e)
        {
            bwManageGameState.RunWorkerAsync();
        }

        private void bwManageGameState_DoWork(object sender, System.ComponentModel.DoWorkEventArgs e)
        {
            HaloGameInterface halo = new HaloGameInterface();

            while (true)
            {
                var allInputs = new Dictionary<ulong, InputMoment>();

                var gs = new GameState();

                bool isRecording = false;
                bool isPlayback = false;

                int lastInputCounter = -1;
                while (true)
                {
                    gs.Counter += 1;
                    gs.Running = halo.IsRunning();
                    while (!gs.Running)
                    {
                        gs.Running = halo.IsRunning();
                        bwManageGameState.ReportProgress(0, gs);
                        Thread.Sleep(100);
                    }

                    byte[] bMapString = new byte[16];
                    byte[] bInputCounter = new byte[4];
                    byte[] bSubLevel = new byte[4];

                    halo.ReadMemory(ADDR_MAP_STRING, ref bMapString);
                    halo.ReadMemory(ADDR_INPUT_TICK, ref bInputCounter);
                    halo.ReadMemory(ADDR_SUBLEVEL_INDICATOR, ref bSubLevel);

                    gs.InputCounter = BitConverter.ToInt32(bInputCounter, 0);
                    gs.SubLevel = BitConverter.ToInt32(bSubLevel, 0);
                    gs.MapString = System.Text.Encoding.UTF8.GetString(bMapString);

                    if (gs.InputCounter == lastInputCounter)
                    {
                        if(gs.InputCounter % 5 == 0)
                            bwManageGameState.ReportProgress(0, gs);
                        continue;
                    }
                    if (lastInputCounter != -1 && gs.InputCounter != 2 && gs.InputCounter > lastInputCounter + 1)
                    {
                        //printf("WARNING: Skipped an input trigger!\n");
                    }
                    lastInputCounter = gs.InputCounter;

                    InputMoment im = new InputMoment();
                    im.InputBuf = new byte[104];

                    byte[] bUpDown = new byte[4];
                    byte[] bLeftRight = new byte[4];
                    byte[] bLeftMouse = new byte[1];
                    byte[] bRightMouse = new byte[1];

                    halo.ReadMemory(ADDR_KEYBOARD_INPUT, ref im.InputBuf);
                    halo.ReadMemory(ADDR_UPDOWNVIEW, ref bUpDown);
                    halo.ReadMemory(ADDR_LEFTRIGHTVIEW, ref bLeftRight);
                    halo.ReadMemory(ADDR_LEFTMOUSE, ref bLeftMouse);
                    halo.ReadMemory(ADDR_RIGHTMOUSE, ref bRightMouse);

                    im.MouseUpDown = BitConverter.ToSingle(bUpDown, 0);
                    im.MouseLeftRight = BitConverter.ToSingle(bLeftRight, 0);
                    im.LeftMouseDown = bLeftMouse[0];
                    im.RightMouseDown = bRightMouse[0];

                    InputKey ik = new InputKey();
                    ik.sublevel = gs.SubLevel;
                    ik.inputCounter = lastInputCounter;

                    byte[] bFrameCounter = new byte[4];
                    halo.ReadMemory(ADDR_FRAMES_SINCE_LEVEL_START, ref bFrameCounter);
                    gs.FrameCounter = (uint)BitConverter.ToInt32(bFrameCounter, 0);

                    if (im.InputBuf[(int)KEYS_INTERNAL.F1] > 1 && !isRecording)
                    {
                        allInputs.Clear();
                        isRecording = false;
                        //printf("Recording Started\n");
                        gs.LastStatus = "Recording Started";
                        im.InputBuf[(int)KEYS_INTERNAL.F1] = 0;
                    }
                    if (im.InputBuf[(int)KEYS_INTERNAL.F2] > 1 && (isRecording || isPlayback))
                    {
                        isRecording = false;
                        isPlayback = false;
                        //printf("Recording/Playback Stopped\n");
                        gs.LastStatus = "Recording/Playback Stopped";
                        im.InputBuf[(int)KEYS_INTERNAL.F2] = 0;
                    }
                    if (im.InputBuf[(int)KEYS_INTERNAL.F3] > 1 && !isPlayback)
                    {
                        isPlayback = true;
                        //printf("Playback Start\n");
                        gs.LastStatus = "Playback Start";
                        im.InputBuf[(int)KEYS_INTERNAL.F3] = 0;
                    }

                    if (lastInputCounter == 0)
                    {
                        if (allInputs.Count == 0)
                        {
                            isRecording = true;
                            //printf("Recording Started\n");
                        }

                        if (gs.FrameCounter <= 1)
                        {
                            //printf("Restart Level\n");
                        }
                        else
                        {
                            //printf("Load Checkpoint\n");
                        }
                    }

                    if (isPlayback && allInputs.ContainsKey(ik.fullKey))
                    {
                        //printf("Replaying Input %u-%d\n", ik.sublevel, ik.inputCounter);
                        //ik.inputCounter -= 1;
                        InputMoment savedIM = allInputs[ik.fullKey];

                        halo.WriteMemory(ADDR_KEYBOARD_INPUT, savedIM.InputBuf);
                        halo.WriteMemory(ADDR_UPDOWNVIEW, BitConverter.GetBytes(savedIM.MouseUpDown));
                        halo.WriteMemory(ADDR_LEFTRIGHTVIEW, BitConverter.GetBytes(savedIM.MouseLeftRight));
                        halo.WriteMemory(ADDR_LEFTMOUSE, BitConverter.GetBytes(savedIM.LeftMouseDown));
                        halo.WriteMemory(ADDR_RIGHTMOUSE, BitConverter.GetBytes(savedIM.RightMouseDown));

                        bwManageGameState.ReportProgress(0, gs);
                        continue;
                    }

                    if (isRecording)
                    {
                        allInputs[ik.fullKey] = im;
                        gs.LastStatus = "Recording";
                    }


                    //printf("%u-%d:", ik.sublevel, lastInputCounter);

                    /*if (ik.inputCounter == 1099) {
                        InputKey ikk;
                        ikk.fullKey = ik.fullKey;
                        ikk.inputCounter = 1088;
                        allInputs[ik.fullKey] = allInputs[ikk.fullKey];
                    }*/

                    //char output[105];
                    //output[104] = 0;
                    /*for (int i = 0; i < sizeof(im.inputBuf); i++) {
                        output[i] = (im.inputBuf[i] == 0) ? 0x20 : KEY_PRINT_CODES[i];
                    }*/
                    //printf("%s\n", output);
                    bwManageGameState.ReportProgress(0, gs);
                }

            }
        }

        private void bwManageGameState_ProgressChanged(object sender, System.ComponentModel.ProgressChangedEventArgs e)
        {
            var state = (GameState)e.UserState;

            lblGameRunning.Text = state.Running ? "YES" : "NO";

            lblFrameCounter.Text = state.FrameCounter.ToString();
            lblInputCounter.Text = state.InputCounter.ToString();
            lblMapString.Text = state.MapString;
            lblSubLevel.Text = state.SubLevel.ToString();
            lblCounter.Text = state.Counter.ToString();
            lblLastStatus.Text = state.LastStatus;
        }

    }
}
