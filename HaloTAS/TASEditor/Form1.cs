using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace TASEditor
{
    public partial class Form1 : Form
    {
        OpenFileDialog ofd = new OpenFileDialog
        {
            DefaultExt = "bin",
            InitialDirectory = @"C:\Halo",
            Filter = @"Halo Binary Logs (*.hbin)|*.hbin",
            RestoreDirectory = true,
            CheckFileExists = true
        };

        private List<InputMoment> currentInputMoments = new List<InputMoment>();

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            dgvInputs.AutoGenerateColumns = true;
            dgvInputs.AutoSize = true;
        }

        private void btnImport_Click(object sender, EventArgs e)
        {
            if (ofd.ShowDialog() != DialogResult.OK)
                return;
            if (string.IsNullOrWhiteSpace(ofd.FileName))
                return;

            LoadHBIN(ofd.OpenFile());

            //var list = new BindingList<InputMoment>(currentInputMoments);
            //inputMomentBindingSource.DataSource = list;

            var dt = new DataTable();
            dt.Columns.Add("CheckpointId", typeof(uint));
            dt.Columns.Add("Tick", typeof(uint));
            //dt.Columns.Add("InputBuffer", typeof(byte[]));
            dt.Columns.Add("MouseX", typeof(int));
            dt.Columns.Add("MouseY", typeof(int));
            dt.Columns.Add("LookDirection", typeof(Vector3));
            dt.Columns.Add("LeftMouse", typeof(byte));
            dt.Columns.Add("RightMouse", typeof(byte));

            foreach (var input in currentInputMoments)
            {
                dt.Rows.Add(input.CheckpointId, input.Tick, input.MouseX, input.MouseY, input.LookDirection, input.LeftMouse, input.RightMouse);
            }

            dgvInputs.DataSource = dt;

        }

        private void LoadHBIN(Stream hbinFileStream)
        {
            currentInputMoments.Clear();

            const int structSize = 136;
            byte[] buf = new byte[structSize];
            while (hbinFileStream.Read(buf, 0, structSize) != 0)
            {
                InputMoment im = new InputMoment
                {
                    CheckpointId = BitConverter.ToUInt32(buf, 0),
                    Tick = BitConverter.ToUInt32(buf, 4),
                    MouseX = BitConverter.ToInt32(buf, 112),
                    MouseY = BitConverter.ToInt32(buf, 116),
                    LookDirection = new Vector3()
                    {
                        X = BitConverter.ToSingle(buf, 120),
                        Y = BitConverter.ToSingle(buf, 124),
                        Z = BitConverter.ToSingle(buf, 128),
                    },
                    LeftMouse = buf[132],
                    RightMouse = buf[133]
                };

                Array.Copy(buf, 8, im.InputBuffer, 0, 104);

                currentInputMoments.Add(im);
            }
        }
    }
}
