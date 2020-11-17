import unittest
import oceanofcode


class TestPointsMethods(unittest.TestCase):
    def test_avg_2(self):
        a = oceanofcode.avg_point([oceanofcode.Point(0, 0), oceanofcode.Point(0, 2)])
        self.assertEqual(a, oceanofcode.Point(0, 1))

    def test_avg_4(self):
        a = oceanofcode.avg_point(
            [oceanofcode.Point(0, 0), oceanofcode.Point(0, 5), oceanofcode.Point(5, 0), oceanofcode.Point(5, 5)])
        self.assertEqual(a, oceanofcode.Point(2, 2))

    def test_add_points(self):
        p1 = oceanofcode.Point(3, 4)
        p2 = oceanofcode.Point(-4, 6)
        self.assertEqual(oceanofcode.add_point(p1, p2), oceanofcode.Point(-1, 10))


class TestGuessTarget(unittest.TestCase):
    m = oceanofcode.Map(15, 15)

    def test_small_move(self):
        self.m.dn, self.m.ds, self.m.de, self.m.dw = 0, 4, 0, 0
        self.m.islands = []
        self.assertIsNone(self.m.guess_target())

    def test_medium_move(self):
        self.m.dn, self.m.ds, self.m.de, self.m.dw = 0, 0, 6, 0
        self.m.islands = []
        self.assertEqual(self.m.guess_target(), oceanofcode.Point(4, 7))

    def test_big_move(self):
        self.m.dn, self.m.ds, self.m.de, self.m.dw = 11, 0, 0, 10
        self.m.islands = []
        self.assertEqual(self.m.guess_target(), oceanofcode.Point(12, 12))

    def test_with_path(self):
        self.m.dn, self.m.ds, self.m.de, self.m.dw = 0, 11, 5, 0
        self.m.islands = []
        self.m.his_path = 11 * ['S'] + 5 * ['E']
        self.assertEqual(self.m.guess_target(), oceanofcode.Point(9, 13))

    def test_with_islands(self):
        self.m.dn, self.m.ds, self.m.de, self.m.dw = 0, 11, 5, 0
        self.m.islands = [oceanofcode.Point(7, 7), oceanofcode.Point(7, 8)]
        self.m.his_path = 11 * ['S'] + 5 * ['E']
        self.assertEqual(self.m.guess_target(), oceanofcode.Point(7, 13))


class TestReadOrders(unittest.TestCase):
    m = oceanofcode.Map()

    def test_move_only(self):
        self.m.read_orders('MOVE S')
        self.assertEqual(self.m.dx, 1)

    def test_wrong_move(self):
        with self.assertRaises(ValueError):
            self.m.read_orders('MOVE N TORPEDO')

    def test_surface(self):
        self.m.read_orders('SURFACE 6')
        self.assertEqual(self.m.target, oceanofcode.Point(12, 7))
        self.assertEqual(self.m.dx, 0)

    def test_torpedo(self):
        self.m.read_orders('TORPEDO 6 4')
        self.assertEqual(self.m.bomb.x, 6)
        self.assertEqual(self.m.bomb.y, 4)

    def test_sonar(self):
        self.m.read_orders('SONAR 9')
        self.assertIsNone(self.m.bomb)

    def test_silence(self):
        self.m.his_path = ['N']
        self.m.read_orders('SILENCE')
        self.assertEqual(len(self.m.his_path), 3)
        self.assertEqual(self.m.his_path[-1], 'N')


class TestInBounds(unittest.TestCase):
    m = oceanofcode.Map(3, 3)

    def test_in(self):
        p = oceanofcode.Point(1, 2)
        self.assertTrue(self.m.in_bounds(p))

    def test_out(self):
        p = oceanofcode.Point(3, 2)
        self.assertFalse(self.m.in_bounds(p))


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


class TestSurfaceCenter(unittest.TestCase):
    m = oceanofcode.Map()

    def test_1(self):
        p = self.m.surface_center(1)
        self.assertEqual(p, oceanofcode.Point(2, 2))

    def test_2(self):
        p = self.m.surface_center(2)
        self.assertEqual(p, oceanofcode.Point(7, 2))

    def test_3(self):
        p = self.m.surface_center(3)
        self.assertEqual(p, oceanofcode.Point(12, 2))

    def test_4(self):
        p = self.m.surface_center(4)
        self.assertEqual(p, oceanofcode.Point(2, 7))

    def test_5(self):
        p = self.m.surface_center(5)
        self.assertEqual(p, oceanofcode.Point(7, 7))

    def test_6(self):
        p = self.m.surface_center(6)
        self.assertEqual(p, oceanofcode.Point(12, 7))

    def test_7(self):
        p = self.m.surface_center(7)
        self.assertEqual(p, oceanofcode.Point(2, 12))

    def test_8(self):
        p = self.m.surface_center(8)
        self.assertEqual(p, oceanofcode.Point(7, 12))

    def test_9(self):
        p = self.m.surface_center(9)
        self.assertEqual(p, oceanofcode.Point(12, 12))


class TestTorpedoRange(unittest.TestCase):
    p = oceanofcode.Point(0, 0)
    m = oceanofcode.Map()

    def test_islands(self):
        self.m.islands.append(oceanofcode.Point(1, 1))
        r = self.m.torpedo_range(self.p)
        self.assertEqual(len(r), 39)
        self.assertIn(oceanofcode.Point(-3, 0), r)
        self.assertIn(oceanofcode.Point(3, 1), r)
        self.assertIn(oceanofcode.Point(2, 2), r)
        self.assertIn(oceanofcode.Point(1, -1), r)
        self.assertNotIn(oceanofcode.Point(1, 1), r)

    def test_range(self):
        self.m.islands = []
        r = self.m.torpedo_range(self.p)
        self.assertEqual(len(r), 40)
        self.assertIn(oceanofcode.Point(-4, 0), r)
        self.assertIn(oceanofcode.Point(3, 0), r)
        self.assertIn(oceanofcode.Point(0, 2), r)
        self.assertIn(oceanofcode.Point(0, -1), r)


class TestTorpedoImpact(unittest.TestCase):
    m = oceanofcode.Map(5,5)

    def test_nominal_impact(self):
        r = self.m.torpedo_impact(oceanofcode.Point(2,2))
        self.assertEqual(len(r), 9)
        self.assertIn(oceanofcode.Point(1, 1), r)
        self.assertIn(oceanofcode.Point(2, 2), r)
        self.assertNotIn(oceanofcode.Point(4, 1), r)

    def test_bound_impact(self):
        r = self.m.torpedo_impact(oceanofcode.Point(0,0))
        self.assertEqual(len(r), 4)


class TestShift(unittest.TestCase):
    m = oceanofcode.Map(15, 15)

    def test_simple(self):
        self.m.moi = oceanofcode.Player(0, 4, 7, 6, 0, 4, 0, -1)
        self.m.target = oceanofcode.Point(5, 5)
        self.assertEqual(self.m.shift(oceanofcode.Point(4, 6)), oceanofcode.Point(4, 4))
    def test_on_me(self):
        self.m.moi = oceanofcode.Player(0, 8, 4, 6, 0, 4, 0, -1)
        self.m.target = oceanofcode.Point(9, 4)
        self.assertEqual(self.m.shift(oceanofcode.Point(9, 4)), oceanofcode.Point(11, 4))


class TestAStar(unittest.TestCase):
    m = oceanofcode.Map(w=15, h=15)

    def test_dist(self):
        self.m.islands = []
        path = self.m.a_star(oceanofcode.Point(1, 1), oceanofcode.Point(7, 1))
        self.assertEqual(len(path), 6)

    def test_not_possible(self):
        self.m.islands = [oceanofcode.Point(7, y) for y in range(self.m.h)]
        path = self.m.a_star(oceanofcode.Point(1, 1), oceanofcode.Point(9, 3))
        self.assertEqual(len(path), 0)


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

    def test_same_position(self):
        self.assertEqual(self.m.direction(oceanofcode.Point(1, 1), oceanofcode.Point(1, 1)), 'W')

    def test_wide_north(self):
        self.assertEqual(self.m.direction(oceanofcode.Point(5, 7), oceanofcode.Point(0, 0)), 'N')

class TestSurfaceVoisine(unittest.TestCase):
    m = oceanofcode.Map(15, 15)
    def test_nominal(self):
        self.m.target = oceanofcode.Point(13,9)
        self.assertEqual(self.m.surface_voisine(), oceanofcode.Point(12,12))

    def test_in_center(self):
        self.m.target = oceanofcode.Point(7,7)
        self.assertEqual(self.m.surface_voisine(), oceanofcode.Point(7,2))

if __name__ == '__main__':
    unittest.main()
