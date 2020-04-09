import unittest
import oceanofcode


class TestTorpedoiRange(unittest.TestCase):
    def test_range(self):
        p = oceanofcode.Point(0, 0)
        m = oceanofcode.Map()
        r = m.torpedo_range(p)
        self.assertEqual(len(r), 40)
        self.assertIn(oceanofcode.Point(-4, 0), r)
        self.assertIn(oceanofcode.Point(3, 0), r)
        self.assertIn(oceanofcode.Point(0, 2), r)
        self.assertIn(oceanofcode.Point(0, -1), r)

    def test_islands(self):
        p = oceanofcode.Point(0, 0)
        m = oceanofcode.Map()
        m.islands.append(oceanofcode.Point(1, 1))
        r = m.torpedo_range(p)
        self.assertEqual(len(r), 39)
        self.assertIn(oceanofcode.Point(-3, 0), r)
        self.assertIn(oceanofcode.Point(3, 1), r)
        self.assertIn(oceanofcode.Point(2, 2), r)
        self.assertIn(oceanofcode.Point(1, -1), r)
        self.assertNotIn(oceanofcode.Point(1, 1), r)


class TestTorpedoImpact(unittest.TestCase):
    def test_impact(self):
        p = oceanofcode.Point(0, 0)
        m = oceanofcode.Map()
        r = m.torpedo_impact(p)
        self.assertEqual(len(r), 9)
        self.assertIn(oceanofcode.Point(-1, -1), r)
        self.assertIn(oceanofcode.Point(0, 0), r)
        self.assertNotIn(oceanofcode.Point(2, 1), r)


class TestSurface(unittest.TestCase):
    m = oceanofcode.Map()

    def test_1(self):
        self.assertEqual(self.m.surface(oceanofcode.Point(0, 0)), 1)

    def test_2(self):
        self.assertEqual(self.m.surface(oceanofcode.Point(6, 1)), 2)

    def test_3(self):
        self.assertEqual(self.m.surface(oceanofcode.Point(12, 3)), 3)

    def test_4(self):
        self.assertEqual(self.m.surface(oceanofcode.Point(2, 5)), 4)

    def test_5(self):
        self.assertEqual(self.m.surface(oceanofcode.Point(7, 6)), 5)

    def test_6(self):
        self.assertEqual(self.m.surface(oceanofcode.Point(11, 9)), 6)

    def test_7(self):
        self.assertEqual(self.m.surface(oceanofcode.Point(4, 13)), 7)

    def test_8(self):
        self.assertEqual(self.m.surface(oceanofcode.Point(8, 10)), 8)

    def test_9(self):
        self.assertEqual(self.m.surface(oceanofcode.Point(14, 14)), 9)

    def test_10(self):
        with self.assertRaises(ValueError):
            self.m.surface(oceanofcode.Point(-1, 0))


class TestNeighbor(unittest.TestCase):
    m = oceanofcode.Map(w=15, h=15)

    def test_middle(self):
        n = self.m.neighbors(oceanofcode.Point(7, 7))
        self.assertEqual(len(n), 4)
        self.assertIn(oceanofcode.Point(6, 7), n)
        self.assertIn(oceanofcode.Point(8, 7), n)
        self.assertIn(oceanofcode.Point(7, 6), n)
        self.assertIn(oceanofcode.Point(7, 8), n)

    def test_bottom_left(self):
        n = self.m.neighbors(oceanofcode.Point(0, 0))
        self.assertEqual(len(n), 2)
        self.assertIn(oceanofcode.Point(0, 1), n)
        self.assertIn(oceanofcode.Point(1, 0), n)

    def test_bottom_right(self):
        n = self.m.neighbors(oceanofcode.Point(14, 0))
        self.assertEqual(len(n), 2)
        self.assertIn(oceanofcode.Point(14, 1), n)
        self.assertIn(oceanofcode.Point(13, 0), n)

    def test_up_left(self):
        n = self.m.neighbors(oceanofcode.Point(0, 14))
        self.assertEqual(len(n), 2)
        self.assertIn(oceanofcode.Point(1, 14), n)
        self.assertIn(oceanofcode.Point(0, 13), n)

    def test_up_right(self):
        n = self.m.neighbors(oceanofcode.Point(14, 14))
        self.assertEqual(len(n), 2)
        self.assertIn(oceanofcode.Point(13, 14), n)
        self.assertIn(oceanofcode.Point(14, 13), n)

    def test_with_island(self):
        self.m.islands.append(oceanofcode.Point(13, 14))
        n = self.m.neighbors(oceanofcode.Point(14, 14))
        self.assertEqual(len(n), 1)
        self.assertIn(oceanofcode.Point(14, 13), n)


class TestDirection(unittest.TestCase):
    m = oceanofcode.Map()

    def test_north(self):
        self.assertEqual(self.m.direction(oceanofcode.Point(1, 1), oceanofcode.Point(1, 0)), 'N')

    def test_south(self):
        self.assertEqual(self.m.direction(oceanofcode.Point(1, 1), oceanofcode.Point(1, 2)), 'S')

    def test_east(self):
        self.assertEqual(self.m.direction(oceanofcode.Point(1, 1), oceanofcode.Point(2, 1)), 'E')

    def test_west(self):
        self.assertEqual(self.m.direction(oceanofcode.Point(1, 1), oceanofcode.Point(0, 1)), 'W')

    def test_error(self):
        with self.assertRaises(ValueError):
            self.m.direction(oceanofcode.Point(1, 1), oceanofcode.Point(10, 1))


if __name__ == '__main__':
    unittest.main()
