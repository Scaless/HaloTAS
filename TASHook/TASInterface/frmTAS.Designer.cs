namespace TASInterface
{
    partial class frmTAS
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.bwManageGameState = new System.ComponentModel.BackgroundWorker();
            this.label1 = new System.Windows.Forms.Label();
            this.lblGameRunning = new System.Windows.Forms.Label();
            this.lblMapString = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.lblInputCounter = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.lblFrameCounter = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.lblSubLevel = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.lblCounter = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.lblLastStatus = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // bwManageGameState
            // 
            this.bwManageGameState.WorkerReportsProgress = true;
            this.bwManageGameState.WorkerSupportsCancellation = true;
            this.bwManageGameState.DoWork += new System.ComponentModel.DoWorkEventHandler(this.bwManageGameState_DoWork);
            this.bwManageGameState.ProgressChanged += new System.ComponentModel.ProgressChangedEventHandler(this.bwManageGameState_ProgressChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(76, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Halo running?:";
            // 
            // lblGameRunning
            // 
            this.lblGameRunning.AutoSize = true;
            this.lblGameRunning.Location = new System.Drawing.Point(94, 9);
            this.lblGameRunning.Name = "lblGameRunning";
            this.lblGameRunning.Size = new System.Drawing.Size(44, 13);
            this.lblGameRunning.TabIndex = 2;
            this.lblGameRunning.Text = "Yes/No";
            // 
            // lblMapString
            // 
            this.lblMapString.AutoSize = true;
            this.lblMapString.Location = new System.Drawing.Point(94, 43);
            this.lblMapString.Name = "lblMapString";
            this.lblMapString.Size = new System.Drawing.Size(65, 13);
            this.lblMapString.TabIndex = 4;
            this.lblMapString.Text = "lblMapString";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(12, 43);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(58, 13);
            this.label3.TabIndex = 3;
            this.label3.Text = "MapString:";
            // 
            // lblInputCounter
            // 
            this.lblInputCounter.AutoSize = true;
            this.lblInputCounter.Location = new System.Drawing.Point(94, 71);
            this.lblInputCounter.Name = "lblInputCounter";
            this.lblInputCounter.Size = new System.Drawing.Size(78, 13);
            this.lblInputCounter.TabIndex = 6;
            this.lblInputCounter.Text = "lblInputCounter";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(12, 71);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(71, 13);
            this.label4.TabIndex = 5;
            this.label4.Text = "InputCounter:";
            // 
            // lblFrameCounter
            // 
            this.lblFrameCounter.AutoSize = true;
            this.lblFrameCounter.Location = new System.Drawing.Point(94, 98);
            this.lblFrameCounter.Name = "lblFrameCounter";
            this.lblFrameCounter.Size = new System.Drawing.Size(83, 13);
            this.lblFrameCounter.TabIndex = 8;
            this.lblFrameCounter.Text = "lblFrameCounter";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(12, 98);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(76, 13);
            this.label6.TabIndex = 7;
            this.label6.Text = "FrameCounter:";
            // 
            // lblSubLevel
            // 
            this.lblSubLevel.AutoSize = true;
            this.lblSubLevel.Location = new System.Drawing.Point(94, 125);
            this.lblSubLevel.Name = "lblSubLevel";
            this.lblSubLevel.Size = new System.Drawing.Size(62, 13);
            this.lblSubLevel.TabIndex = 10;
            this.lblSubLevel.Text = "lblSubLevel";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(12, 125);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(55, 13);
            this.label8.TabIndex = 9;
            this.label8.Text = "SubLevel:";
            // 
            // lblCounter
            // 
            this.lblCounter.AutoSize = true;
            this.lblCounter.Location = new System.Drawing.Point(94, 150);
            this.lblCounter.Name = "lblCounter";
            this.lblCounter.Size = new System.Drawing.Size(54, 13);
            this.lblCounter.TabIndex = 12;
            this.lblCounter.Text = "lblCounter";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(12, 150);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(47, 13);
            this.label5.TabIndex = 11;
            this.label5.Text = "Counter:";
            // 
            // lblLastStatus
            // 
            this.lblLastStatus.AutoSize = true;
            this.lblLastStatus.Location = new System.Drawing.Point(304, 9);
            this.lblLastStatus.Name = "lblLastStatus";
            this.lblLastStatus.Size = new System.Drawing.Size(67, 13);
            this.lblLastStatus.TabIndex = 14;
            this.lblLastStatus.Text = "lblLastStatus";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(222, 9);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(63, 13);
            this.label7.TabIndex = 13;
            this.label7.Text = "Last Status:";
            // 
            // frmTAS
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(811, 548);
            this.Controls.Add(this.lblLastStatus);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.lblCounter);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.lblSubLevel);
            this.Controls.Add(this.label8);
            this.Controls.Add(this.lblFrameCounter);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.lblInputCounter);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.lblMapString);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.lblGameRunning);
            this.Controls.Add(this.label1);
            this.Name = "frmTAS";
            this.Text = "Halo TAS";
            this.Load += new System.EventHandler(this.frmTAS_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.ComponentModel.BackgroundWorker bwManageGameState;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label lblGameRunning;
        private System.Windows.Forms.Label lblMapString;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label lblInputCounter;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label lblFrameCounter;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label lblSubLevel;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Label lblCounter;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label lblLastStatus;
        private System.Windows.Forms.Label label7;
    }
}

