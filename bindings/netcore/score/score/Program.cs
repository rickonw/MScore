using Newtonsoft.Json;
using Quartz;
using Quartz.Impl;
using System;
using System.Collections.Specialized;
using System.Threading.Tasks;

namespace score
{
    public class HelloJob : IJob
    {
        public Task Execute(IJobExecutionContext context)
        {
            return Console.Out.WriteLineAsync("Greetings from HelloJob!");
        }
    }
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("Time Job Start");
            RunProgram().GetAwaiter().GetResult();
            Console.WriteLine("Hello World!");
            Console.Read();
        }

        private static async Task RunProgram()
        {
            try
            {
                // Grab the Scheduler instance from the Factory  
                NameValueCollection props = new NameValueCollection
                {
                    { "quartz.serializer.type", "binary" }
                };
                StdSchedulerFactory factory = new StdSchedulerFactory(props);
                IScheduler scheduler = await factory.GetScheduler();

                // 启动任务调度器  
                await scheduler.Start();

                // 定义一个 Job  
                IJobDetail job = JobBuilder.Create<HelloJob>()
                    .WithIdentity("job1", "group1")
                    .Build();

                long ticksPerSecond = 10000000;
                TimeSpan interval = new TimeSpan(ticksPerSecond/1000);
                ISimpleTrigger trigger = (ISimpleTrigger)TriggerBuilder.Create()
                    .WithIdentity("trigger1") // 给任务一个名字  
                    .StartAt(DateTime.Now) // 设置任务开始时间  
                    .ForJob("job1", "group1") //给任务指定一个分组  
                    .WithSimpleSchedule(x => x
                    .WithInterval(interval)  //循环的时间 1秒1次 
                    .RepeatForever())
                    .Build();
                

                // 等待执行任务  
                await scheduler.ScheduleJob(job, trigger);

                // some sleep to show what's happening  
                //await Task.Delay(TimeSpan.FromMilliseconds(2000));  
            }
            catch (SchedulerException se)
            {
                await Console.Error.WriteLineAsync(se.ToString());
            }
        }
    }
}
