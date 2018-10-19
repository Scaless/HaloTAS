using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace TASEditor
{
    class InputMoment
    {
        public uint CheckpointId;
        public uint Tick;
        public byte[] InputBuffer;
        public int MouseX, MouseY;
        public Vector3 LookDirection;
        public byte LeftMouse, RightMouse;

        public InputMoment()
        {
            InputBuffer = new byte[104];
        }

        public override string ToString()
        {
            int keysPressedCount = InputBuffer.Count(key => key > 0);

            return $"Tick: {Tick} \tPressed Keys: {keysPressedCount} \tMouseMoved: {{{MouseX},{MouseY}}}" +
                   $" \tLookDir: {{{LookDirection.ToString()}}} \tMouseButtons: {{{LeftMouse},{RightMouse}}}";
        }
    }
}
