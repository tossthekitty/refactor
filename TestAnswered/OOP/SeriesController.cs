using System;
using System.Collections.Generic;

namespace OOP
{
    public abstract class SeriesController : ISeriesControllerTest
    {
            public bool ReadData() {
                // read data using controller specific protocol
                // ......
                // add some data to a queue (for test only)
                _data.Enqueue(new Tuple<string, double>( Identity,DoubleValue));
                return _data.Count < 3;
            }

            public bool GetData(out string name, out double value) {
                name = string.Empty;
                value = 0;
                if (_data.Count == 0)
                    return false;
                Tuple<string, double> data = _data.Dequeue();
                name = data.Item1;
                value = data.Item2;
                return true;
            }

            private readonly Queue<Tuple<string, double>> _data = new Queue<Tuple<string, double>>();

            public virtual string Identity
            {
                get
                {
                    throw new NotImplementedException();
                }
                set
                {
                    throw new NotImplementedException();
                }
            }

            public virtual double DoubleValue
            {
                get
                {
                    throw new NotImplementedException();
                }
                set
                {
                    throw new NotImplementedException();
                }
            }
    };    
}
