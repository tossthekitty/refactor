using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace OOP {
	/// <summary>
	/// Improve the following code.  Hint: You can apply object oriented programming principles  
	/// </summary>
	class Program {

		class Series3Controller {
			public void Connect(string address) {
				// connect to series 3 controller
			}
			public bool ReadData() {
				// read data using controller specific protocol
				// ......
				// add some data to a queue (for test only)
				_Data.Enqueue(new Tuple<string, double>("Item1", 1));
				return _Data.Count < 3;
			}
			public bool GetData(out string Name, out double Value) {
				Name = string.Empty;
				Value = 0;
				if (_Data.Count == 0)
					return false;
				Tuple<string, double> data = _Data.Dequeue();
				Name = data.Item1;
				Value = data.Item2;
				return true;
			}
			public Queue<Tuple<string, double>> _Data = new Queue<Tuple<string, double>>();
		};

		class Series5Controller {
			public void Connect(string address) {
				// connect to series 5 controller
			}
			public bool ReadData() {
				// read data using controller specific protocol
				// ......
				// add some data to a queue (for test only)
				_Data.Enqueue(new Tuple<string, double>("Item2", 2));
				return _Data.Count < 3;
			}
			public bool GetData(out string Name, out double Value) {
				Name = string.Empty;
				Value = 0;
				if (_Data.Count == 0)
					return false;
				Tuple<string, double> data = _Data.Dequeue();
				Name = data.Item1;
				Value = data.Item2;
				return true;
			}
			public Queue<Tuple<string, double>> _Data = new Queue<Tuple<string, double>>();
		}

		class Series6Controller {
			public void Connect(string address) {
				// connect to series 6 controller
			}
			public bool ReadData() {
				// read data using controller specific protocol
				// ......
				// add some data to a queue (for test only)
				_Data.Enqueue(new Tuple<string, double>("Item3", 3));
				return _Data.Count < 3;
			}
			public bool GetData(out string Name, out double Value) {
				Name = string.Empty;
				Value = 0;
				if (_Data.Count == 0)
					return false;
				Tuple<string, double> data = _Data.Dequeue();
				Name = data.Item1;
				Value = data.Item2;
				return true;
			}
			public Queue<Tuple<string, double>> _Data = new Queue<Tuple<string, double>>();
		}
		/// <summary>
		/// Communcation server
		/// </summary>
		class CommServer {
			public Series3Controller S3controller = new Series3Controller();
			public Series5Controller S5controller = new Series5Controller();
			public Series6Controller S6controller = new Series6Controller();

			public void Connect(string address) {
				if (address.StartsWith("s3."))
					S3controller.Connect(address);
				else if (address.StartsWith("s5."))
					S5controller.Connect(address);
				else if (address.StartsWith("s6."))
					S6controller.Connect(address);
			}
			public void ReadData() {
				while (S3controller.ReadData());
				while (S5controller.ReadData());
				while (S6controller.ReadData());
			}
			public void ProcessData() {
				string Name;
				double Value;
				while (S3controller.GetData(out Name, out Value))
					SendToClient(Name, Value);
				while (S5controller.GetData(out Name, out Value))
					SendToClient(Name, Value);
				while (S6controller.GetData(out Name, out Value))
					SendToClient(Name, Value);
			}

			public void SendToClient(string Name, double Value) {
				// send data to client
			}
		}

		static void Main(string[] args) {
			CommServer server = new CommServer();

			server.ReadData();

			server.ProcessData();
		}

	};
}