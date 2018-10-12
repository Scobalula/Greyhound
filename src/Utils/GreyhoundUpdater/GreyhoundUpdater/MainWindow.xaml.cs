// ------------------------------------------------------------------------------
// Primitive Updater for apps released on Github - Made for Greyhound - by Scobalula
// Copyright(c) 2018 Scobalula/Philip
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// ------------------------------------------------------------------------------
using System;
using System.Diagnostics;
using System.IO;
using System.IO.Compression;
using System.Windows;
using System.Threading;
using System.ComponentModel;
using System.Net;
using System.Reflection;
using Octokit;

namespace GreyhoundUpdater
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        /// <summary>
        /// Latest Github Release Asset
        /// </summary>
        ReleaseAsset LatestReleaseAsset { get; set; }

        /// <summary>
        /// Gets Application Directory
        /// </summary>
        public string ApplicationDirectory { get { return Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location); } }

        /// <summary>
        /// Name of the Github Repo to Query
        /// </summary>
        public string GithubRepoName { get; set; }

        /// <summary>
        /// Name of the User Account that owns the Repo
        /// </summary>
        public string GithubUserName { get; set; }

        /// <summary>
        /// Name of the Application we're updating
        /// </summary>
        public string ApplicationName { get; set; }

        /// <summary>
        /// Name of the assembly of the Application we're updating
        /// </summary>
        public string AssemblyName { get; set; }

        /// <summary>
        /// Whether we want to download via the Client or Open a browser
        /// </summary>
        public bool DownloadViaClient = true;

        /// <summary>
        /// A tracker variable to check if we're updating
        /// </summary>
        public bool Updating = false;

        /// <summary>
        /// Main Entry Point for WPF
        /// </summary>
        public MainWindow()
        {
            // Check command line arguments
            if(!GetCommandLineArguments())
            {
                MessageBox.Show("This application is intended to be opened by its parent program.", "Updater", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                Close();
            }
            // Try grab and open the window, if we fail, exit
            try
            {
                // Query Github Releases
                var client = new GitHubClient(new ProductHeaderValue(ApplicationName));
                var releases = client.Repository.Release.GetAll(GithubUserName, GithubRepoName);
                var latestRelease = releases.Result[0];
                // Query File Version
                var fileInfo = FileVersionInfo.GetVersionInfo(AssemblyName);
                // Convert to Version Objects for comparison
                var currentVersion = new Version(fileInfo.FileVersion);
                var remoteVersion = new Version(latestRelease.TagName);
                // Set Latest Release Asset
                LatestReleaseAsset = latestRelease.Assets[0];
                // Check current Version against Remote
                if(currentVersion < remoteVersion)
                {
                    InitializeComponent();
                    // Set Version
                    LatestVersion.Content = String.Format("Update {0}", remoteVersion);
                    // Set Titles
                    Title = ApplicationName + " Update";
                    TitleLabel.Content = ApplicationName + " Update";
                    // Append change log
                    Changes.AppendText(latestRelease.Body);
                }
                else
                {
                    Close();
                }
            }
            catch
            {
               Close();
            }
        }

        /// <summary>
        /// Gets Update Info from Command Line Arguments
        /// </summary>
        /// <returns></returns>
        private bool GetCommandLineArguments()
        {
            // Get arguments
            string[] args = Environment.GetCommandLineArgs();
            // Check did we receive the correct number of arguments (we want 5, so we check 5 since we're including this assembly)
            if (args.Length < 6)
                return false;
            // Assign data
            GithubUserName    = args[1];
            GithubRepoName    = args[2];
            ApplicationName   = args[3];
            AssemblyName      = args[4];
            DownloadViaClient = args[5] == "true";
            // Got it
            return true;
        }

        /// <summary>
        /// Cancel Button Logic (Closes Window)
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Cancel_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

        /// <summary>
        /// Download Button Logic
        /// </summary>
        private void DownloadNow_Click(object sender, RoutedEventArgs e)
        {
            // Check whether we want to download now, or open a browser
            if (DownloadViaClient)
            {
                // Change to Updating
                Updating = true;
                // Change Views
                ChangelogGrid.Visibility = Visibility.Hidden;
                DownloadNow.Visibility = Visibility.Hidden;
                Cancel.Visibility = Visibility.Hidden;
                ProgressGrid.Visibility = Visibility.Visible;
                // Get Processes matching Greyhound
                Process[] processes = Process.GetProcessesByName(AssemblyName.Split('.')[0]);
                // Loop and Kill any instances of Greyhound
                foreach (var process in processes)
                    process.Kill();
                // Initiate the Download
                InitiateDownload();
            }
            else
            {
                Process.Start(LatestReleaseAsset.BrowserDownloadUrl);
                Close();
            }
        }

        /// <summary>
        /// Initiates Github Asset Download
        /// </summary>
        private void InitiateDownload()
        {
            // Create and set download methods and data
            new Thread(() => {
                WebClient client = new WebClient();
                client.DownloadProgressChanged += new DownloadProgressChangedEventHandler(UpdateProgress);
                client.DownloadFileCompleted += new AsyncCompletedEventHandler(DownloadComplete);
                client.DownloadFileAsync(new Uri(LatestReleaseAsset.BrowserDownloadUrl), "Update.zip");
            }).Start();
        }

        /// <summary>
        /// Updates Progress Bar during Download
        /// </summary>
        private void UpdateProgress(object sender, DownloadProgressChangedEventArgs e)
        {
            Dispatcher.BeginInvoke(new Action(() => 
            {
                Progress.Value = int.Parse(Math.Truncate(double.Parse(e.BytesReceived.ToString()) / double.Parse(e.TotalBytesToReceive.ToString()) * 100).ToString());
            }));
        }

        /// <summary>
        /// Extracts and relaunches Greyhound on Download complete
        /// </summary>
        private void DownloadComplete(object sender, AsyncCompletedEventArgs e)
        {
            // Invoke UI Changes and launch Finalize method
            Dispatcher.BeginInvoke(new Action(() =>
            {
                ProgressInfo.Content = "Finalizing....";
                Progress.IsIndeterminate = true;
                FinalizeUpdate();
            }));
        }

        /// <summary>
        /// Finalizes the Update by Unpacking the assets and relaunching Greyhound
        /// </summary>
        private void FinalizeUpdate()
        {
            // Check if Update Zip was actually downloaded
            if(File.Exists("Update.zip"))
            {
                // Try unpack, we should not fail unless permission issues, etc. occur.
                try
                {
                    using (ZipArchive archive = new ZipArchive(new FileStream("Update.zip", System.IO.FileMode.Open)))
                    {
                        foreach (var entry in archive.Entries)
                        {
                            if (!String.IsNullOrWhiteSpace(entry.Name) && entry.Name != Path.GetFileName(Process.GetCurrentProcess().MainModule.FileName))
                            {
                                string resultingFile = Path.Combine(ApplicationDirectory, entry.FullName);

                                Directory.CreateDirectory(Path.GetDirectoryName(resultingFile));

                                entry.ExtractToFile(resultingFile, true);
                            }
                        }
                    }
                }
                catch
                {
                    Updating = false;
                    MessageBox.Show("Update failed. Could not unpack Update.zip, potentially corrupt download.", "ERROR", MessageBoxButton.OK, MessageBoxImage.Error);
                    Close();
                }
            }
            else
            {
                Updating = false;
                MessageBox.Show("Update failed. Could not find Update.zip.", "ERROR", MessageBoxButton.OK, MessageBoxImage.Error);
                Close();
            }
            // Try launch Greyhound again
            try
            {
                Process.Start("Greyhound.exe");
            }
            catch
            {
                Updating = false;
                MessageBox.Show("Failed to execute Greyhound.", "ERROR", MessageBoxButton.OK, MessageBoxImage.Error);
                Close();
            }
            // Set back
            Updating = false;
            // Delete Update File
            if(File.Exists("Update.zip"))
            {
                try
                {
                    File.Delete("Update.zip");
                }
                catch
                {
                    // Mission failed, we'll get 'em next time
                    MessageBox.Show("Failed to delete Update.zip, please delete it manually", "ERROR", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                }
            }
            // Delay the exit
            DelayExit();
        }

        /// <summary>
        /// Delays exit to give time for Greyhound to start
        /// </summary>
        private void DelayExit()
        {
            new Thread(() => 
            {
                Thread.Sleep(1000);
                Dispatcher.BeginInvoke(new Action(() =>
                {
                    Close();
                }));
            }).Start();
        }

        /// <summary>
        /// Checks if we're updating on close
        /// </summary>
        private void Window_Closing(object sender, CancelEventArgs e)
        {
            if (Updating)
                e.Cancel = true;
        }
    }
}
