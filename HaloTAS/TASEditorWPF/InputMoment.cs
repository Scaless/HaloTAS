using System.Linq;

namespace TASEditorWPF
{
    public class InputMoment
    {
        public uint CheckpointId;
        public uint Tick;
        public byte[] InputBuffer;
        public float MouseYaw; // Left-Right
        public float MousePitch; // Up-Down

        public InputMoment()
        {
            InputBuffer = new byte[104];
        }

        public override string ToString()
        {
            int keysPressedCount = InputBuffer.Count(key => key > 0);

            return $"Tick: {Tick} \tPressed Keys: {keysPressedCount} \tMouseMove: {{{MouseYaw},{MousePitch}}}";
        }
    }
}
