using System;
using System.Collections.Generic;

namespace OOP {
    /// <summary>
    /// Improve the following code.  Hint: You can apply object oriented programming principles  
    /// </summary>
    class Program {

        class Series3Controller : SeriesController
        {
            private const string _identity = "Item1";
            private double _value = 1;
            public override string Identity { get { return _identity; } }
            public override double DoubleValue { get { return _value++; } }
        };

        class Series5Controller : SeriesController
        {
            private const string _identity = "Item2";
            private double _value = 1;
            public override string Identity { get { return _identity; } }
            public override double DoubleValue { get { return _value++; } }
        }

        class Series6Controller : SeriesController
        {
            private const string _identity = "Item3";
            private double _value = 1;
            public override string Identity { get { return _identity; } }
            public override double DoubleValue { get { return _value++; } }
        }
        /// <summary>
        /// Communcation server
        /// </summary>
        class CommServer {
            private readonly List<SeriesController> _controllers;
            
            public CommServer()
            {
                _controllers = new List<SeriesController>
                {
                    new Series3Controller(),
                    new Series5Controller(),
                    new Series6Controller()
                };
            }
 
            public void ReadData()
            {
                _controllers.ForEach(t =>
                {
                    while (t.ReadData())
                    {
                    }
                });
            }

            public void ProcessData() {
                string name;
                double value;
                _controllers.ForEach(t =>
                {
                    while (t.GetData(out name, out value))
                    {
                        SendToClient(name, value);
                    }
                });
            }

            private void SendToClient(string name, double value) {
                Console.WriteLine("{0} ... {1}",name,value);
            }
        }

        static void Main(string[] args) {
            CommServer server = new CommServer();
            server.ReadData();
            server.ProcessData();
            Console.ReadKey();
        }
    };
}