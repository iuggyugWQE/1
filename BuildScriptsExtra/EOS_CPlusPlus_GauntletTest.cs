using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using AutomationTool;
using UnrealBuildTool;
using Gauntlet;
using System.IO;
using System.IO.Compression;
using System.Text.RegularExpressions;
using System.Reflection;
using System.Net.Sockets;
using System.Net.NetworkInformation;
using System.Diagnostics;

#if IS_UE5
namespace UnrealGame
{
    public class EOSGauntletP2PNetworkingTest : EOSGauntletNetworkTest<UnrealTestConfig>
#else
namespace UE4Game
{
    public class EOSGauntletP2PNetworkingTest : EOSGauntletNetworkTest<UE4TestConfig>
#endif
    {
        public EOSGauntletP2PNetworkingTest(UnrealTestContext InContext) : base(InContext)
        {
        }

#if IS_UE5
        public override UnrealTestConfig GetConfiguration()
        {
#else
        public override UE4TestConfig GetConfiguration()
        {
#endif
            var Config = base.GetConfiguration();

            var GeneratedSessionGUID = Guid.NewGuid().ToString();

            List<UnrealTestRole> Roles = new List<UnrealTestRole>(
                new UnrealTestRole[0]
                .Concat(RequireAllowedRoles(Config, UnrealTargetRole.Client, UnrealTargetPlatform.Win64, 4))
            );
            int ListeningRoleIndex = (int)Math.Floor((double)new Random().Next(0, Roles.Count));

            Log.Info("{0} {1} [ LOG] {2}", "Gauntlet".PadRight(30), "".PadRight(12), "Using role #" + ListeningRoleIndex + " (" + Roles[ListeningRoleIndex].PlatformOverride.ToString() + ") as listen server.");

            for (int i = 0; i < Roles.Count; i++)
            {
                var ClientRole = Roles[i];
                ClientRole.CommandLine = "-ini:Engine:[EpicOnlineServices]:AuthenticationGraph=Anonymous -ini:Engine:[EpicOnlineServices]:EnableAntiCheat=False -ini:Engine:[PacketHandlerComponents]:EncryptionComponent= -ini:Engine:[EpicOnlineServices]:EnableIdentityChecksOnListenServers=False -ini:Engine:[EpicOnlineServices]:EnableTrustedDedicatedServers=False -ini:Engine:[EpicOnlineServices]:EnableAutomaticEncryptionOnTrustedDedicatedServers=False -ini:Engine:[EpicOnlineServices]:EnableSanctionChecks=False -requireeos -eosgauntletsessionguid=" + GeneratedSessionGUID + " -eosroleid=" + i + " -emulateeosshipping" + GetPlatformArgs(ClientRole);
                ClientRole.MapOverride = "/Game/ExampleOSS/Common/EmptyMap";
                if (ListeningRoleIndex == i)
                {
                    ClientRole.Controllers.Add("EOSGauntletRunListenServerTest");
                }
                else
                {
                    ClientRole.Controllers.Add("EOSGauntletConnectToSessionTest");
                }
            }
            
            return Config;
        }
    }

    /********* TEST BASE CLASSES *********/

    public abstract class EOSGauntletNetworkTest<T> : EOSGauntletTest<T> where T : UnrealTestConfiguration, new()
    {
        private Dictionary<UnrealSessionInstance.RoleInstance, bool> RolesThatConnectedOrListened;

        public EOSGauntletNetworkTest(UnrealTestContext InContext) : base(InContext)
        {
        }

        public string AutomationTestName { get; protected set; } = "OnlineSubsystemEOS.Gauntlet.P2PNetworking";

        public override bool StartTest(int Pass, int InNumPasses)
        {
            if (base.StartTest(Pass, InNumPasses))
            {
                this.RolesThatConnectedOrListened = TestInstance.RunningRoles.ToDictionary(k => k, v => false);
                return true;
            }

            return false;
        }

        public override void ProcessLogLine(UnrealSessionInstance.RoleInstance Role, string LogLine)
        {
            if (LogLine.Contains("Connected to server, scheduling disconnect") || LogLine.Contains("Scheduling map change every twenty seconds"))
            {
                this.RolesThatConnectedOrListened[Role] = true;
            }
        }
        
        protected virtual bool DidNetworkTestPass()
        {
            return !(this.RolesThatConnectedOrListened.Any(kv => !kv.Value) || GetUnrealTestResult() != TestResult.Passed);
        }

#if IS_UE5
        protected override void StopTest(bool WasCancelled)
#else
        public override void StopTest(bool WasCancelled)
#endif
		{
            if (!DidNetworkTestPass())
            {
                foreach (var MissingKV in this.RolesThatConnectedOrListened.Where(kv => !kv.Value))
                {
                    Log.Info("{0} {1} [ LOG] {2}", "Gauntlet".PadRight(30), "".PadRight(12), "Role " + MissingKV.Key.Role.Platform.ToString() + " did not listen or connect.");
                }
                try
                {
                    Console.ForegroundColor = ConsoleColor.Red;
                    Log.Info("{0} {1} [FAIL] {2}", "Gauntlet".PadRight(30), "".PadRight(12), AutomationTestName);
                }
                finally
                {
                    Console.ResetColor();
                }
                SetUnrealTestResult(TestResult.Failed);
            }
            else
            {
                try
                {
                    Console.ForegroundColor = ConsoleColor.Green;
                    Log.Info("{0} {1} [PASS] {2}", "Gauntlet".PadRight(30), "".PadRight(12), AutomationTestName);
                }
                finally
                {
                    Console.ResetColor();
                }
                SetUnrealTestResult(TestResult.Passed);
            }

            base.StopTest(WasCancelled);
        }
    }

#if IS_UE5
    public class EOSGauntletTest : EOSGauntletTest<UnrealTestConfig>
#else
    public class EOSGauntletTest : EOSGauntletTest<UE4TestConfig>
#endif
    {
        public EOSGauntletTest(UnrealTestContext InContext) : base(InContext)
        {
        }
    }

    public abstract class EOSGauntletTest<T> : UnrealTestNode<T> where T : UnrealTestConfiguration, new()
    {
        public EOSGauntletTest(UnrealTestContext InContext) : base(InContext)
        {
        }

        private Dictionary<IAppInstance, int> LogLinesLastTick = new Dictionary<IAppInstance, int>();
        private Regex TestStartedRegex = new Regex(@"LogAutomationController.+Test Started. Name={(.+?)}");
        private Regex TestCompleteRegex = new Regex(@"LogAutomationController.+Test Completed. Result={(.+?)}\s+Name={(.+?)}\s+Path={(.+?)}");

        protected IEnumerable<UnrealTestRole> RequireAllowedRoles(T Config, UnrealTargetRole Role, UnrealTargetPlatform Platform, int Num)
        {
            if (PlatformAllowed(Platform))
            {
                return Config.RequireRoles(Role, Platform, Num);
            }
            
            return new List<UnrealTestRole>();
        }

        protected bool PlatformAllowed(UnrealTargetPlatform? InPlatform)
        {
            if (!InPlatform.HasValue)
            {
                return true;
            }
            var SkipPlatform = Environment.GetEnvironmentVariable("SKIP_PLATFORM_" + InPlatform.Value);
            if (SkipPlatform == "true")
            {
                Console.WriteLine("warning: Skipping platform {0} in test, because the SKIP_PLATFORM_{1} variable is set to true.", InPlatform.Value, InPlatform.Value);
                return false;
            }
            if (Environment.GetEnvironmentVariable("USE_PLATFORM_ALLOWLIST") == "true")
            {
                if (Environment.GetEnvironmentVariable("ALLOW_PLATFORM_" + InPlatform.Value) != "true")
                {
                    Console.WriteLine("info: Skipping platform {0} in test, because the allowlist is turned on and this platform isn't allowed.", InPlatform.Value);
                    return false;
                }
            }
            return true;
        }

        protected string GetPlatformArgs(UnrealTestRole Role)
        {
            if (Role.PlatformOverride == UnrealTargetPlatform.Win64)
            {
                return " -nullrhi";
            }

            return string.Empty;
        }

		public override bool StartTest(int Pass, int InNumPasses)
		{
			if (!base.StartTest(Pass, InNumPasses))
			{
				return false;
			}

			LogLinesLastTick = new Dictionary<IAppInstance, int>();
			return true;
		}

        public virtual void ProcessLogLine(UnrealSessionInstance.RoleInstance Role, string LogLine)
        {
        }

        protected virtual HashSet<string> GetKnownTestFailures()
        {
            return new HashSet<string>();
        }

        public override void TickTest()
        {
            var knownTestFailures = GetKnownTestFailures();

            foreach (UnrealSessionInstance.RoleInstance Role in TestInstance.RunningRoles)
            {
                IAppInstance App = Role.AppInstance;
                UnrealLogParser LogParser = new UnrealLogParser(App.StdOut == null ? "" : App.StdOut);

                string PlatformString = App.Device.Platform.HasValue ? App.Device.Platform.Value.ToString() : "";
                if (Role.Role.RoleType.IsServer())
                {
                    PlatformString = "Server";
                }

                if (!LogLinesLastTick.ContainsKey(App))
                {
                    LogLinesLastTick[App] = 0;
                } 

                string[] Lines = LogParser.Content.Split(new[] { '\n' }, StringSplitOptions.RemoveEmptyEntries);
                int LogLines = Lines.Length;

                for (int i = LogLinesLastTick[App]; i < LogLines; i++)
                {
                    this.ProcessLogLine(Role, Lines[i].TrimEnd());

                    if (Lines[i].Contains("]LogEOSGauntlet:"))
                    {
                        Log.Info("{0} {1} [ LOG] {2}", App.Device.Name.PadRight(30), PlatformString.PadRight(12), Lines[i].TrimEnd());
                    }

                    if (Lines[i].Contains("Error: You are not using the EOS networking driver, so the host address can not be set correctly during the CreateSession or UpdateSession call."))
                    {
                        // This is a misconfiguration, and the test must immediately end.
                        try
                        {
                            Console.ForegroundColor = ConsoleColor.Red;
                            Log.Info("{0} {1} {2} {3}", App.Device.Name.PadRight(30), PlatformString.PadRight(12), "[FAIL]", Lines[i].TrimEnd());
                        }
                        finally
                        {
                            Console.ResetColor();
                        }
                        MarkTestComplete();
                        SetUnrealTestResult(TestResult.Failed);
                        continue;
                    }

                    Match LineMatch = TestCompleteRegex.Match(Lines[i]);
                    if (LineMatch != null && LineMatch.Success)
                    {
                        bool bPassed = LineMatch.Groups[1].ToString().ToLower() == "passed" ? true : false;
                        string Name = LineMatch.Groups[2].ToString();
                        string LongName = LineMatch.Groups[3].ToString();

                        bool bAllowedFailure = false;
                        if (!bPassed && App.Device.Platform.HasValue)
                        {
                            // Check to see if this test is allowed to fail, as per our "known failing tests" dictionary.
                            if (knownTestFailures.Contains(LongName.Trim()))
                            {
                                bAllowedFailure = true;
                            }
                        }

                        try
                        {
                            Console.ForegroundColor = bPassed ? ConsoleColor.Green : (bAllowedFailure ? ConsoleColor.Yellow : ConsoleColor.Red);
                            Log.Info("{0} {1} {2} {3}", App.Device.Name.PadRight(30), PlatformString.PadRight(12), bPassed ? "[PASS]" : (bAllowedFailure ? "[KNWN]" : "[FAIL]"), LongName);
                        }
                        finally
                        {
                            Console.ResetColor();
                        }

                        if (!bPassed && !bAllowedFailure)
                        {
                            // Get all the logs that were emitted during the test so we can pass them back to the
                            // user to diagnose the issue.
                            int TestStartedLine = 0;
                            for (int ii = i; ii > 0; ii--)
                            {
                                Match StartMatch = TestStartedRegex.Match(Lines[ii]);
                                if (StartMatch.Success && StartMatch.Groups[1].ToString() == Name) 
                                {
                                    TestStartedLine = ii;
                                    break;
                                }
                            }
                            for (int ii = TestStartedLine; ii < LogLines; ii++)
                            {
                                Log.Info("{0} {1} [ LOG] {2}", App.Device.Name.PadRight(30), PlatformString.PadRight(12), Lines[ii].TrimEnd());
                            }

                            // Immediately fail so we aren't waiting to get this result.
                            MarkTestComplete();
                            SetUnrealTestResult(TestResult.Failed);
                        }
                    }
                }

                LogLinesLastTick[App] = LogLines;
            }

            base.TickTest();
        }

        public override bool IsTestRunning()
        {
			var MissingRoles = FindAnyMissingRoles().ToList();

            if (MissingRoles.Count == 0)
            {
                // Nothing has finished yet.
                return true;
            }

            foreach (var Role in MissingRoles)
            {
                if (Role.AppInstance.ExitCode != 0)
                {
                    Log.Info(
                        "Exiting {0} because role {1} has non-zero exit code of {2}", 
                        Name, 
                        Role.ToString(),
                        Role.AppInstance.ExitCode);
                    return false;
                }
            }

			if (MissingRoles.Count == TestInstance.RunningRoles.Count())
			{
                // Everything has exited either normally or otherwise.
			    Log.Info(
                    "Ending {0} because all roles have exited", 
                    Name);
                return false;
            }

            if (MissingRoles.Any(x => x.AppInstance.ExitCode == 0 && (x.AppInstance.CommandLine.Contains("EOSGauntletRunListenServerTest") || x.AppInstance.CommandLine.Contains("EOSGauntletRunDedicatedServerTest"))))
            {
                // In this case, the test ends because the listen/dedicated server has cleanly exited.
			    Log.Info(
                    "Ending {0} because the listen/dedicated server has cleanly shut down", 
                    Name);
                return false;
            }

            // Tests are still running.
            return true;
        }

#if !IS_UE5
		public override void CreateReport(TestResult Result, UnrealTestContext Context, UnrealBuildSource Build, IEnumerable<UnrealRoleArtifacts> Artifacts, string InArtifactPath)
		{
            Log.Info("-- CreateReport called!");
            Log.Info("-- Result=" + Result);
            Log.Info("-- Context=" + Context);
            Log.Info("-- Build=" + Build);
            Log.Info("-- Artifacts=" + Artifacts);
            Log.Info("-- InArtifactPath=" + InArtifactPath);

            if (Artifacts != null)
            {
                foreach (var RoleArtifact in Artifacts)
                {
                    Log.Info("---- Got artifact:");
                    Log.Info("----   SessionRole.RoleType=" + RoleArtifact.SessionRole.RoleType);
                    Log.Info("----   SessionRole.Platform=" + RoleArtifact.SessionRole.Platform);
                    Log.Info("----   SessionRole.Configuration=" + RoleArtifact.SessionRole.Configuration);
                    Log.Info("----   ArtifactPath=" + RoleArtifact.ArtifactPath);
                    Log.Info("----   LogPath=" + RoleArtifact.LogPath);
                    Log.Info("----   LogParser=" + RoleArtifact.LogParser);
                    if (RoleArtifact.LogSummary == null)
                    {
                        Log.Info("----   LogSummary=(null)");
                    }
                    else
                    {
                        Log.Info("----   LogSummary.BuildInfo=" + RoleArtifact.LogSummary.BuildInfo);
                        Log.Info("----   LogSummary.PlatformInfo=" + RoleArtifact.LogSummary.PlatformInfo);
                        Log.Info("----   LogSummary.Warnings=" + RoleArtifact.LogSummary.Warnings);
                        Log.Info("----   LogSummary.Errors=" + RoleArtifact.LogSummary.Errors);
                        Log.Info("----   LogSummary.FatalError=" + RoleArtifact.LogSummary.FatalError);
                        Log.Info("----   LogSummary.Ensures=" + RoleArtifact.LogSummary.Ensures);
                        Log.Info("----   LogSummary.LineCount=" + RoleArtifact.LogSummary.LineCount);
                        Log.Info("----   LogSummary.EngineInitialized=" + RoleArtifact.LogSummary.EngineInitialized);
                        Log.Info("----   LogSummary.RequestedExit=" + RoleArtifact.LogSummary.RequestedExit);
                        Log.Info("----   LogSummary.RequestedExitReason=" + RoleArtifact.LogSummary.RequestedExitReason);
                        Log.Info("----   LogSummary.HasTestExitCode=" + RoleArtifact.LogSummary.HasTestExitCode);
                        Log.Info("----   LogSummary.TestExitCode=" + RoleArtifact.LogSummary.TestExitCode);
                        Log.Info("----   LogSummary.HasAbnormalExit=" + RoleArtifact.LogSummary.HasAbnormalExit);
                    }

                    DirectoryInfo RoleDir = new DirectoryInfo(RoleArtifact.ArtifactPath);

                    IEnumerable<FileInfo> ArtifactFiles = RoleDir.EnumerateFiles("*.*", SearchOption.AllDirectories);

                    // user may not have cleared paths between runs, so throw away anything that's older than 2m
                    ArtifactFiles = ArtifactFiles.Where(F => (DateTime.Now - F.LastWriteTime).TotalMinutes < 2);

                    foreach (var File in ArtifactFiles)
                    {
                        Log.Info("------ Got artifact file: " + File.FullName);
                    }

                    IEnumerable<FileInfo> LogFiles = ArtifactFiles.Where(F => F.Extension.Equals(".log", StringComparison.OrdinalIgnoreCase));

                    foreach (FileInfo LogFile in LogFiles)
                    {
                        Log.Info("------ Got artifact file (log): " + LogFile.FullName);

                        AutomationLogParser LogParser = new AutomationLogParser(File.ReadAllText(LogFile.FullName));
                        
                        foreach (var TestResult in LogParser.GetResults())
                        {
                            Log.Info("-------- Test result:");
                            Log.Info("--------   DisplayName=" + TestResult.DisplayName);
                            Log.Info("--------   TestName=" + TestResult.TestName);
                            Log.Info("--------   Completed=" + TestResult.Completed);
                            Log.Info("--------   Passed=" + TestResult.Passed);
                            Log.Info("--------   WarningEvents=");
                            foreach (var Ev in TestResult.WarningEvents)
                            {
                                Log.Info("--------     " + Ev);
                            }
                            Log.Info("--------   ErrorEvents=");
                            foreach (var Ev in TestResult.ErrorEvents)
                            {
                                Log.Info("--------     " + Ev);
                            }
                            Log.Info("--------   FullName=" + TestResult.FullName);
                        }
                    }
                }
            }
		}
#endif

        public static string GetLocalIpAddress() 
        {
            foreach (var netI in NetworkInterface.GetAllNetworkInterfaces())
            {
                if (netI.NetworkInterfaceType != NetworkInterfaceType.Wireless80211 &&
                    (netI.NetworkInterfaceType != NetworkInterfaceType.Ethernet ||
                     netI.OperationalStatus != OperationalStatus.Up)) continue;
                foreach (var uniIpAddrInfo in netI.GetIPProperties().UnicastAddresses.Where(x => netI.GetIPProperties().GatewayAddresses.Count > 0))
                {

                    if (uniIpAddrInfo.Address.AddressFamily == AddressFamily.InterNetwork &&
                        uniIpAddrInfo.AddressPreferredLifetime != uint.MaxValue)
                        return uniIpAddrInfo.Address.ToString();
                }
            }
            Log.Info("You local IPv4 address couldn't be found...");
            return null;
        }

#if IS_UE5
		protected override UnrealProcessResult GetExitCodeAndReason(StopReason InReason, UnrealLog InLog, UnrealRoleArtifacts InArtifacts, out string ExitReason, out int ExitCode)
#else
		protected override int GetExitCodeAndReason(UnrealRoleArtifacts InArtifacts, out string ExitReason)
#endif
        {
            if (InArtifacts.SessionRole.Platform == UnrealTargetPlatform.IOS &&
                GetTestResult() == TestResult.Passed)
            {
                ExitReason = "Ignoring exit code, because IOS does not correctly handle exits.";
#if IS_UE5
                ExitCode = 0;
                return UnrealProcessResult.ExitOk;
#else
                return 0;
#endif
            }

#if IS_UE5
            return base.GetExitCodeAndReason(InReason, InLog, InArtifacts, out ExitReason, out ExitCode);
#else
            return base.GetExitCodeAndReason(InArtifacts, out ExitReason);
#endif
        }
    }
}