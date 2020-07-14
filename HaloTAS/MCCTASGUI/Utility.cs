using System;
using System.Collections.Generic;
using System.Text;

namespace MCCTASGUI
{
    public class EnumUtils<T> where T : struct, IConvertible
    {
        public static int Count
        {
            get
            {
                if (!typeof(T).IsEnum)
                    throw new ArgumentException("T must be an enumerated type");

                return Enum.GetNames(typeof(T)).Length;
            }
        }
    }

}
