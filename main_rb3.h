static const char *rgss3_exe =
/* TODO: this is only for developing purpose. */
/* We want to remove it when it becomes unnecessary. */
 "def warn_unimplemented(name)\n"
 "  unimplemented_warnings = eval(\"$unimplemented_warnings ||= {}\")\n"
 "  if !unimplemented_warnings[name]\n"
 "    unimplemented_warnings[name] = true\n"
#ifdef __DEBUG__
 "    $stderr.puts \"unimplemented: #{name}\"\n"
#endif
 "  end\n"
 "end\n"
 "class Font\n"
 "  @@default_name = \"VL Gothic\"\n"
 "  @@default_size = 24\n"
 "  @@default_bold = false\n"
 "  @@default_italic = false\n"
 "  @@default_outline = true\n"
 "  @@default_shadow = false\n"
 "  @@default_color = Color.new(255, 255, 255, 255)\n"
 "  @@default_out_color = Color.new(0, 0, 0, 128)\n"
 "  def self.default_name\n"
 "    @@default_name\n"
 "  end\n"
 "  def self.default_name=(default_name)\n"
 "    @@default_name = default_name\n"
 "  end\n"
 "  def self.default_size\n"
 "    @@default_size\n"
 "  end\n"
 "  def self.default_size=(default_size)\n"
 "    @@default_size = default_size\n"
 "  end\n"
 "  def self.default_bold\n"
 "    @@default_bold\n"
 "  end\n"
 "  def self.default_bold=(default_bold)\n"
 "    @@default_bold = default_bold\n"
 "  end\n"
 "  def self.default_italic\n"
 "    @@default_italic\n"
 "  end\n"
 "  def self.default_italic=(default_italic)\n"
 "    @@default_italic = default_italic\n"
 "  end\n"
 "  def self.default_outline\n"
 "    @@default_outline\n"
 "  end\n"
 "  def self.default_outline=(default_outline)\n"
 "    @@default_outline = default_outline\n"
 "  end\n"
 "  def self.default_shadow\n"
 "    @@default_shadow\n"
 "  end\n"
 "  def self.default_shadow=(default_shadow)\n"
 "    @@default_shadow = default_shadow\n"
 "  end\n"
 "  def self.default_color\n"
 "    @@default_color\n"
 "  end\n"
 "  def self.default_color=(default_color)\n"
 "    @@default_color = default_color\n"
 "  end\n"
 "  def self.default_out_color\n"
 "    @@default_out_color\n"
 "  end\n"
 "  def self.default_out_color=(default_out_color)\n"
 "    @@default_out_color = default_out_color\n"
 "  end\n"
 "end\n"
 "\n"
 "module RPG\n"
 "  class Map\n"
 "    attr_accessor :autoplay_bgm, :autoplay_bgs, :bgm, :bgs, :data, :encounter_list, :encounter_step, :events, :height, :width\n"
 "    attr_accessor :disable_dashing, :parallax_loop_x, :parallax_loop_y, :parallax_name, :parallax_show, :parallax_sx, :parallax_sy, :scroll_type\n"
 "    attr_accessor :battleback1_name, :battleback2_name, :display_name, :note, :specify_battleback, :tileset_id\n"
 "    def initialize(width, height)\n"
 "      @autoplay_bgm = false\n"
 "      @autoplay_bgs = false\n"
 "      @encounter_list = []\n"
 "      @encounter_step = 30\n"
 "      @events = {}\n"
 "      @height = height\n"
 "      @width = width\n"
 "      @bgm = RPG::BGM.new\n"
 "      @bgs = RPG::BGS.new(\"\", 80)\n"
 "      @disable_dashing = false\n"
 "      @parallax_loop_x = false\n"
 "      @parallax_loop_y = false\n"
 "      @parallax_name = \"\"\n"
 "      @parallax_show = false\n"
 "      @parallax_sx = 0\n"
 "      @parallax_sy = 0\n"
 "      @scroll_type = 0\n"
 "      @battleback1_name = \"\"\n"
 "      @battleback2_name = \"\"\n"
 "      @display_name = \"\"\n"
 "      @note = \"\"\n"
 "      @specify_battleback = false\n"
 "      @data = Table.new(width, height, 4)\n"
/* Really??? */
 "      @tileset_id = 1\n"
 "    end\n"
 "    class Encounter\n"
 "      attr_accessor :region_set, :troop_id, :weight\n"
 "      def initialize\n"
 "        @region_set = []\n"
 "        @troop_id = 1\n"
 "        @weight = 10\n"
 "      end\n"
 "    end\n"
 "  end\n"
 "  class MapInfo\n"
 "    attr_accessor :expanded, :name, :order, :parent_id, :scroll_x, :scroll_y\n"
 "    def initialize\n"
 "      @expanded = false\n"
 "      @name = \"\"\n"
 "      @order = 0\n"
 "      @parent_id = 0\n"
 "      @scroll_x = 0\n"
 "      @scroll_y = 0\n"
 "    end\n"
 "  end\n"
 "  class Event\n"
 "    attr_accessor :id, :name, :pages, :x, :y\n"
 "    def initialize(x, y)\n"
 "      @id = 0\n"
 "      @name = \"\"\n"
 "      @pages = [RPG::Event::Page.new]\n"
 "      @x = x\n"
 "      @y = y\n"
 "    end\n"
 "    class Page\n"
 "      attr_accessor :condition, :direction_fix, :graphic, :list, :move_frequency, :move_route, :move_speed, :move_type, :step_anime, :through, :trigger, :walk_anime\n"
 "      attr_accessor :priority_type\n"
 "      def initialize\n"
 "        @condition = RPG::Event::Page::Condition.new\n"
 "        @direction_fix = false\n"
 "        @graphic = RPG::Event::Page::Graphic.new\n"
 "        @list = [RPG::EventCommand.new]\n"
 "        @move_frequency = 3\n"
 "        @move_route = RPG::MoveRoute.new\n"
 "        @move_speed = 3\n"
 "        @move_type = 0\n"
 "        @step_anime = false\n"
 "        @through = false\n"
 "        @trigger = 0\n"
 "        @walk_anime = true\n"
 "        @priority_type = 0\n"
 "      end\n"
 "      class Condition\n"
 "        attr_accessor :self_switch_ch, :self_switch_valid, :switch1_id, :switch1_valid, :switch2_id, :switch2_valid, :variable_id, :variable_valid, :variable_value\n"
 "        attr_accessor :actor_id, :actor_valid, :item_id, :item_valid\n"
 "        def initialize\n"
 "          @self_switch_ch = \"A\"\n"
 "          @self_switch_valid = false\n"
 "          @switch1_id = 1\n"
 "          @switch1_valid = false\n"
 "          @switch2_id = 1\n"
 "          @switch2_valid = false\n"
 "          @variable_id = 1\n"
 "          @variable_valid = false\n"
 "          @variable_value = 0\n"
 "          @actor_id = 1\n"
 "          @actor_valid = false\n"
 "          @item_id = 1\n"
 "          @item_valid = false\n"
 "        end\n"
 "      end\n"
 "      class Graphic\n"
 "        attr_accessor :character_name, :direction, :pattern, :tile_id\n"
 "        attr_accessor :character_index\n"
 "        def initialize\n"
 "          @character_name = \"\"\n"
 "          @direction = 2\n"
 "          @pattern = 0\n"
 "          @tile_id = 0\n"
 "          @character_index = 0\n"
 "        end\n"
 "      end\n"
 "    end\n"
 "  end\n"
 "  class EventCommand\n"
 "    attr_accessor :code, :indent, :parameters\n"
 "    def initialize(code = 0, indent = 0, parameters = [])\n"
 "      @code = code\n"
 "      @indent = indent\n"
 "      @parameters = parameters\n"
 "    end\n"
 "  end\n"
 "  class MoveRoute\n"
 "    attr_accessor :list, :repeat, :skippable\n"
 "    attr_accessor :wait\n"
 "    def initialize\n"
 "      @list = [RPG::MoveCommand.new]\n"
 "      @repeat = true\n"
 "      @skippable = false\n"
 "      @wait = false\n"
 "    end\n"
 "  end\n"
 "  class MoveCommand\n"
 "    attr_accessor :code, :parameters\n"
 "    def initialize(code = 0, parameters = [])\n"
 "      @code = code\n"
 "      @parameters = parameters\n"
 "    end\n"
 "  end\n"
 "  class BaseItem\n"
 "    attr_accessor :description, :icon_index, :id, :name, :note\n"
 "    attr_accessor :features\n"
 "    class Feature\n"
 "      attr_accessor :code, :data_id, :value\n"
 "      def initialize(code = 0, data_id = 0, value = 0)\n"
 "        @code = code\n"
 "        @data_id = data_id\n"
 "        @value = value\n"
 "      end\n"
 "    end\n"
 "    def initialize\n"
 "      @description = \"\"\n"
 "      @icon_index = 0\n"
 "      @id = 0\n"
 "      @name = \"\"\n"
 "      @note = \"\"\n"
 "      @features = []\n"
 "    end\n"
 "  end\n"
 "  class Actor < BaseItem\n"
 "    attr_accessor :character_index, :character_name, :class_id, :equips, :face_index, :face_name, :initial_level, :max_level, :nickname\n"
 "    def initialize\n"
 "      super\n"
 "      @character_index = 0\n"
 "      @character_name = \"\"\n"
 "      @class_id = 1\n"
 "      @equips = [0, 0, 0, 0, 0]\n"
 "      @face_index = 0\n"
 "      @face_name = \"\"\n"
 "      @initial_level = 1\n"
 "      @max_level = 99\n"
 "      @nickname = \"\"\n"
 "    end\n"
 "  end\n"
 "  class Class < BaseItem\n"
 "    attr_accessor :exp_params, :learnings, :params\n"
 "    def initialize\n"
 "      super\n"
 "      @exp_params = [30, 20, 30, 30]\n"
 "      @features << RPG::BaseItem::Feature.new(23, 0, 1)\n"
 "      @features << RPG::BaseItem::Feature.new(22, 0, 0.95)\n"
 "      @features << RPG::BaseItem::Feature.new(22, 1, 0.05)\n"
 "      @features << RPG::BaseItem::Feature.new(22, 2, 0.04)\n"
 "      @features << RPG::BaseItem::Feature.new(41, 1, 0)\n"
 "      @features << RPG::BaseItem::Feature.new(51, 1, 0)\n"
 "      @features << RPG::BaseItem::Feature.new(52, 1, 0)\n"
 "      @learnings = []\n"
 "      @params = Table.new(8, 100)\n"
 "      (1..99).each do|level|\n"
 "        @params[0, level] = level * 50 + 400\n"
 "        @params[1, level] = level * 10 + 80\n"
 "        @params[2, level] = (level * 5 + 60) / 4\n"
 "        @params[3, level] = (level * 5 + 60) / 4\n"
 "        @params[4, level] = (level * 5 + 60) / 4\n"
 "        @params[5, level] = (level * 5 + 60) / 4\n"
 "        @params[6, level] = (level * 5 + 60) / 2\n"
 "        @params[7, level] = (level * 5 + 60) / 2\n"
 "      end\n"
 "    end\n"
 "    def exp_for_level(level)\n"
 "      level = level.to_f\n"
 "      a = @exp_params[0].to_f * (level - 1) ** (0.9 + @exp_params[2].to_f / 250) * level * (level + 1)\n"
 "      b = 6 + level ** 2 / 50 / @exp_params[3].to_f\n"
 "      (a / b + (level - 1) * @exp_params[1].to_f).round.to_i\n"
 "    end\n"
 "    class Learning\n"
 "      attr_accessor :level, :note, :skill_id\n"
 "      def initialize\n"
 "        @level = 1\n"
 "        @note = \"\"\n"
 "        @skill_id = 1\n"
 "      end\n"
 "    end\n"
 "  end\n"
 "  class UsableItem < BaseItem\n"
 "    def initialize\n"
 "      super\n"
 "      @animation_id = 0\n"
 "      @damage = RPG::UsableItem::Damage.new\n"
 "      @effects = []\n"
 "      @hit_type = 0\n"
 "      @occasion = 0\n"
 "      @repeats = 1\n"
 "      @scope = 0\n"
 "      @speed = 0\n"
 "      @success_rate = 100\n"
 "      @tp_gain = 0\n"
 "    end\n"
 "    attr_accessor :animation_id, :damage, :effects, :hit_type, :occasion, :repeats, :scope, :speed, :success_rate, :tp_gain\n"
 "    def for_opponent?\n"
 "      [1, 2, 3, 4, 5, 6].include?(@scope)\n"
 "    end\n"
 "    def for_friend?\n"
 "      [7, 8, 9, 10, 11].include?(@scope)\n"
 "    end\n"
 "    def for_dead_friend?\n"
 "      [9, 10].include?(@scope)\n"
 "    end\n"
 "    def for_user?\n"
 "      @scope == 11\n"
 "    end\n"
 "    def for_one?\n"
 "      [1, 3, 7, 9, 11].include?(@scope)\n"
 "    end\n"
 "    def for_random?\n"
 "      [3, 4, 5, 6].include?(@scope)\n"
 "    end\n"
 "    def number_of_targets\n"
 "      [0, 0, 0, 1, 2, 3, 4, 0, 0, 0, 0, 0][@scope]\n"
 "    end\n"
 "    def for_all?\n"
 "      [2, 8, 10].include?(@scope)\n"
 "    end\n"
 "    def need_selection?\n"
 "      [1, 7, 9].include?(@scope)\n"
 "    end\n"
 "    def battle_ok?\n"
 "      [0, 1].include?(@occasion)\n"
 "    end\n"
 "    def menu_ok?\n"
 "      [0, 2].include?(@occasion)\n"
 "    end\n"
 "    def certain?\n"
 "      @hit_type == 0\n"
 "    end\n"
 "    def physical?\n"
 "      @hit_type == 1\n"
 "    end\n"
 "    def magical?\n"
 "      @hit_type == 2\n"
 "    end\n"
 "    class Damage\n"
 "      attr_accessor :critical, :element_id, :formula, :type, :variance\n"
 "      def none?\n"
 "        @type == 0\n"
 "      end\n"
 "      def to_hp?\n"
 "        [1, 3, 5].include?(@type)\n"
 "      end\n"
 "      def to_mp?\n"
 "        [2, 4, 6].include?(@type)\n"
 "      end\n"
 "      def recover?\n"
 "        [3, 4].include?(@type)\n"
 "      end\n"
 "      def drain?\n"
 "        [5, 6].include?(@type)\n"
 "      end\n"
 "      def sign\n"
 "        recover? ? -1 : 1\n"
 "      end\n"
 "      def eval(a, b, v)\n"
 "        [Kernel.eval(@formula), 0].max * self.sign rescue 0\n"
 "      end\n"
 "      def initialize\n"
 "        @critical = false\n"
 "        @element_id = 0\n"
 "        @formula = \"0\"\n"
 "        @type = 0\n"
 "        @variance = 20\n"
 "      end\n"
 "    end\n"
 "    class Effect\n"
 "      attr_accessor :code, :data_id, :value1, :value2\n"
 "      def initialize(code = 0, data_id = 0, value1 = 0, value2 = 0)\n"
 "        @code = code\n"
 "        @data_id = data_id\n"
 "        @value1 = value1\n"
 "        @value2 = value2\n"
 "      end\n"
 "    end\n"
 "  end\n"
 "  class Skill < UsableItem\n"
 "    attr_accessor :message1, :message2, :mp_cost, :required_wtype_id1, :required_wtype_id2, :stype_id, :tp_cost\n"
 "    def initialize\n"
 "      super\n"
 "      @message1 = \"\"\n"
 "      @message2 = \"\"\n"
 "      @mp_cost = 0\n"
 "      @required_wtype_id1 = 0\n"
 "      @required_wtype_id2 = 0\n"
 "      @scope = 1\n"
 "      @stype_id = 1\n"
 "      @tp_cost = 0\n"
 "    end\n"
 "  end\n"
 "  class Item < UsableItem\n"
 "    attr_accessor :consumable, :itype_id, :price\n"
 "    def key_item?\n"
 "      @itype_id == 2\n"
 "    end\n"
 "    def initialize\n"
 "      super\n"
 "      @consumable = true\n"
 "      @itype_id = 1\n"
 "      @price = 0\n"
 "      @scope = 7\n"
 "    end\n"
 "  end\n"
 "  class EquipItem < BaseItem\n"
 "    attr_accessor :etype_id, :params, :price\n"
 "    def initialize\n"
 "      super\n"
 "      @etype_id = 0\n"
 "      @params = [0, 0, 0, 0, 0, 0, 0, 0]\n"
 "      @price = 0\n"
 "    end\n"
 "  end\n"
 "  class Weapon < EquipItem\n"
 "    attr_accessor :animation_id, :wtype_id\n"
 "    def performance\n"
 "      @params[2] + @params[4] + @params.inject(&:+)\n"
 "    end\n"
 "    def initialize\n"
 "      super\n"
 "      @animation_id = 0\n"
 "      @wtype_id = 0\n"
 "      @features << RPG::BaseItem::Feature.new(31, 1, 0)\n"
 "      @features << RPG::BaseItem::Feature.new(22, 0, 0)\n"
 "    end\n"
 "  end\n"
 "  class Armor < EquipItem\n"
 "    attr_accessor :atype_id\n"
 "    def performance\n"
 "      @params[3] + @params[5] + @params.inject(&:+)\n"
 "    end\n"
 "    def initialize\n"
 "      super\n"
 "      @atype_id = 0\n"
 "      @etype_id = 1\n"
 "      @features << RPG::BaseItem::Feature.new(22, 1, 0)\n"
 "    end\n"
 "  end\n"
 "  class Enemy < BaseItem\n"
 "    attr_accessor :actions, :battler_hue, :battler_name, :drop_items, :exp, :gold, :params\n"
 "    def initialize\n"
 "      super\n"
 "      @actions = [RPG::Enemy::Action.new]\n"
 "      @battler_hue = 0\n"
 "      @battler_name = \"\"\n"
 "      @drop_items = [RPG::Enemy::DropItem.new, RPG::Enemy::DropItem.new, RPG::Enemy::DropItem.new]\n"
 "      @exp = 0\n"
 "      @features << RPG::BaseItem::Feature.new(22, 0, 0.95)\n"
 "      @features << RPG::BaseItem::Feature.new(22, 1, 0.05)\n"
 "      @features << RPG::BaseItem::Feature.new(31, 1, 0)\n"
 "      @gold = 0\n"
 "      @params = [100, 0, 10, 10, 10, 10, 10, 10]\n"
 "    end\n"
 "    class Action\n"
 "      attr_accessor :condition_param1, :condition_param2, :condition_type, :rating, :skill_id\n"
 "      def initialize\n"
 "        @condition_param1 = 0\n"
 "        @condition_param2 = 0\n"
 "        @condition_type = 0\n"
 "        @rating = 5\n"
 "        @skill_id = 1\n"
 "      end\n"
 "    end\n"
 "    class DropItem\n"
 "      attr_accessor :data_id, :denominator, :kind\n"
 "      def initialize\n"
 "        @data_id = 1\n"
 "        @denominator = 1\n"
 "        @kind = 0\n"
 "      end\n"
 "    end\n"
 "  end\n"
 "  class State < BaseItem\n"
 "    attr_accessor :auto_removal_timing, :chance_by_damage, :max_turns, :message1, :message2, :message3, :message4, :min_turns, :priority, :remove_at_battle_end, :remove_by_damage, :remove_by_restriction, :remove_by_walking, :restriction, :steps_to_remove\n"
 "    def initialize\n"
 "      super\n"
 "      @auto_removal_timing = 0\n"
 "      @chance_by_damage = 100\n"
 "      @max_turns = 1\n"
 "      @message1 = \"\"\n"
 "      @message2 = \"\"\n"
 "      @message3 = \"\"\n"
 "      @message4 = \"\"\n"
 "      @min_turns = 1\n"
 "      @priority = 50\n"
 "      @remove_at_battle_end = false\n"
 "      @remove_by_damage = false\n"
 "      @remove_by_restriction = false\n"
 "      @remove_by_walking = false\n"
 "      @restriction = 0\n"
 "      @steps_to_remove = 100\n"
 "    end\n"
 "  end\n"
 "  class Troop\n"
 "    attr_accessor :id, :name, :members, :pages\n"
 "    def initialize\n"
 "      @id = 0\n"
 "      @members = []\n"
 "      @name = \"\"\n"
 "      @pages = [RPG::Troop::Page.new]\n"
 "    end\n"
 "    class Member\n"
 "      attr_accessor :enemy_id, :hidden, :x, :y\n"
 "      def initialize\n"
 "        @enemy_id = 1\n"
 "        @hidden = false\n"
 "        @x = 0\n"
 "        @y = 0\n"
 "      end\n"
 "    end\n"
 "    class Page\n"
 "      attr_accessor :condition, :list, :span\n"
 "      def initialize\n"
 "        @condition = RPG::Troop::Page::Condition.new\n"
 "        @list = [RPG::EventCommand.new]\n"
 "        @span = 0\n"
 "      end\n"
 "      class Condition\n"
 "        attr_accessor :turn_ending\n"
 "        attr_accessor :actor_hp, :actor_id, :actor_valid, :enemy_hp, :enemy_index, :enemy_valid, :switch_id, :switch_valid, :turn_a, :turn_b, :turn_valid\n"
 "        def initialize\n"
 "          @actor_hp = 50\n"
 "          @actor_id = 1\n"
 "          @actor_valid = false\n"
 "          @enemy_hp = 50\n"
 "          @enemy_index = 0\n"
 "          @enemy_valid = false\n"
 "          @switch_id = 1\n"
 "          @switch_valid = false\n"
 "          @turn_a = 0\n"
 "          @turn_b = 0\n"
 "          @turn_valid = false\n"
 "          @turn_ending = false\n"
 "        end\n"
 "      end\n"
 "    end\n"
 "  end\n"
 "  class Animation\n"
 "    attr_accessor :frame_max, :frames, :id, :name, :position, :timings\n"
 "    attr_accessor :animation1_hue, :animation1_name, :animation2_hue, :animation2_name\n"
 "    def to_screen?\n"
 "      @position == 3\n"
 "    end\n"
 "    def initialize\n"
 "      @animation1_hue = 0\n"
 "      @animation1_name = \"\"\n"
 "      @animation2_hue = 0\n"
 "      @animation2_name = \"\"\n"
 "      @frame_max = 1\n"
 "      @frames = [RPG::Animation::Frame.new]\n"
 "      @id = 0\n"
 "      @name = \"\"\n"
 "      @position = 1\n"
 "      @timings = []\n"
 "    end\n"
 "    class Frame\n"
 "      attr_accessor :cell_data, :cell_max\n"
 "      def initialize\n"
 "        @cell_data = Table.new(0, 0)\n"
 "        @cell_max = 0\n"
 "      end\n"
 "    end\n"
 "    class Timing\n"
 "      attr_accessor :flash_color, :flash_duration, :flash_scope, :frame, :se\n"
 "      def initialize\n"
 "        @flash_color = Color.new(255, 255, 255, 255)\n"
 "        @flash_duration = 5\n"
 "        @flash_scope = 0\n"
 "        @frame = 0\n"
 "        @se = RPG::SE.new(\"\", 80)\n"
 "      end\n"
 "    end\n"
 "  end\n"
 "  class Tileset\n"
 "    attr_accessor :flags, :id, :mode, :name, :note, :tileset_names\n"
 "    def initialize\n"
 "      @flags = Table.new(0x2000)\n"
 "      @flags[0] = 0x10\n"
 "      (0x0800...0x0B00).each {|i| @flags[i] = 0x0f }\n"
 "      (0x1100...0x2000).each {|i| @flags[i] = 0x0f }\n"
 "      @id = 0\n"
 "      @mode = 1\n"
 "      @name = \"\"\n"
 "      @note = \"\"\n"
 "      @tileset_names = [\"\", \"\", \"\", \"\", \"\", \"\", \"\", \"\", \"\"]\n"
 "    end\n"
 "  end\n"
 "  class CommonEvent\n"
 "    attr_accessor :id, :name, :trigger, :switch_id, :list\n"
 "    def autorun?\n"
 "      @trigger == 1\n"
 "    end\n"
 "    def parallel?\n"
 "      @trigger == 2\n"
 "    end\n"
 "    def initialize\n"
 "      @id = 0\n"
 "      @list = [RPG::EventCommand.new]\n"
 "      @name = \"\"\n"
 "      @switch_id = 1\n"
 "      @trigger = 0\n"
 "    end\n"
 "  end\n"
 "  class System\n"
 "    attr_accessor :airship, :boat, :game_title, :ship, :sounds, :terms, :version_id\n"
 "    attr_accessor :battle_bgm, :battle_end_me, :battler_hue, :battler_name, :edit_map_id, :elements, :gameover_me, :party_members, :start_map_id, :start_x, :start_y, :switches, :test_battlers, :test_troop_id, :title_bgm, :variables\n"
 "    attr_accessor :armor_types, :battleback1_name, :battleback2_name, :currency_unit, :japanese, :opt_display_tp, :opt_draw_title, :opt_extra_exp, :opt_floor_death, :opt_followers, :opt_slip_death, :opt_transparent, :opt_use_midi, :skill_types, :title1_name, :title2_name, :weapon_types, :window_tone\n"
 "    def initialize\n"
 "      @airship = RPG::System::Vehicle.new\n"
 "      @armor_types = [nil, \"\"]\n"
 "      @battle_bgm = RPG::BGM.new\n"
 "      @battle_end_me = RPG::ME.new\n"
 "      @battleback1_name = \"\"\n"
 "      @battleback2_name = \"\"\n"
 "      @battler_hue = 0\n"
 "      @battler_name = \"\"\n"
 "      @boat = RPG::System::Vehicle.new\n"
 "      @currency_unit = \"\"\n"
 "      @edit_map_id = 1\n"
 "      @elements = [nil, \"\"]\n"
 "      @game_title = \"\"\n"
 "      @gameover_me = RPG::ME.new\n"
 "      @japanese = true\n"
 "      @opt_display_tp = true\n"
 "      @opt_draw_title = true\n"
 "      @opt_extra_exp = false\n"
 "      @opt_floor_death = false\n"
 "      @opt_followers = true\n"
 "      @opt_slip_death = false\n"
 "      @opt_transparent = false\n"
 "      @opt_use_midi = false\n"
 "      @party_members = [1]\n"
 "      @ship = RPG::System::Vehicle.new\n"
 "      @skill_types = [nil, \"\"]\n"
 "      @sounds = Array.new(24) { RPG::SE.new }\n"
 "      @start_map_id = 1\n"
 "      @start_x = 0\n"
 "      @start_y = 0\n"
 "      @switches = [nil, \"\"]\n"
 "      @terms = RPG::System::Terms.new\n"
 "      @test_battlers = []\n"
 "      @test_troop_id = 1\n"
 "      @title1_name = \"\"\n"
 "      @title2_name = \"\"\n"
 "      @title_bgm = RPG::BGM.new\n"
 "      @variables = [nil, \"\"]\n"
 "      @version_id = 0\n"
 "      @weapon_types = [nil, \"\"]\n"
 "      @window_tone = Tone.new(0, 0, 0, 0)\n"
 "    end\n"
 "    class Terms\n"
 "      attr_accessor :basic, :commands, :etypes, :params\n"
 "      def initialize\n"
 "        @basic = [\"\", \"\", \"\", \"\", \"\", \"\", \"\", \"\"]\n"
 "        @commands = [\"\", \"\", \"\", \"\", \"\", \"\", \"\", \"\", \"\", \"\", \"\", \"\", \"\", \"\", \"\", \"\", \"\", \"\", \"\", \"\", \"\", \"\", \"\"]\n"
 "        @etypes = [\"\", \"\", \"\", \"\", \"\"]\n"
 "        @params = [\"\", \"\", \"\", \"\", \"\", \"\", \"\", \"\"]\n"
 "      end\n"
 "    end\n"
 "    class TestBattler\n"
 "      attr_accessor :actor_id, :level\n"
 "      attr_accessor :equips\n"
 "      def initialize\n"
 "        @actor_id = 1\n"
 "        @level = 1\n"
 "        @equips = [0, 0, 0, 0, 0]\n"
 "      end\n"
 "    end\n"
 "    class Vehicle\n"
 "      attr_accessor :bgm, :character_index, :character_name, :start_map_id, :start_x, :start_y\n"
 "      def initialize\n"
 "        @bgm = RPG::BGM.new\n"
 "        @character_index = 0\n"
 "        @character_name = \"\"\n"
 "        @start_map_id = 0\n"
 "        @start_x = 0\n"
 "        @start_y = 0\n"
 "      end\n"
 "    end\n"
 "  end\n"
 "  class AudioFile\n"
 "    attr_accessor :name, :pitch, :volume\n"
 "    def initialize(name = \"\", volume = 100, pitch = 100)\n"
 "      @name = name\n"
 "      @pitch = pitch\n"
 "      @volume = volume\n"
 "    end\n"
 "  end\n"
 "  class BGM < AudioFile\n"
 "    attr_accessor :pos\n"
 "    def replay\n"
 "      play(@pos)\n"
 "    end\n"
 "    def play(pos=0)\n"
 "      if @name == \"\"\n"
 "        Audio.bgm_stop\n"
 "        @@last = BGM.new\n"
 "      else\n"
 "        Audio.bgm_play(\"Audio/BGM/#@name\", @volume, @pitch, pos)\n"
 "        @@last = self.clone\n"
 "      end\n"
 "    end\n"
 "    def self.last\n"
 "      @@last.pos = Audio.bgm_pos\n"
 "      @@last\n"
 "    end\n"
 "    def self.stop\n"
 "      Audio.bgm_stop\n"
 "      @@last = BGM.new\n"
 "    end\n"
 "    def self.fade(time)\n"
 "      Audio.bgm_fade(time)\n"
 "      @@last = BGM.new\n"
 "    end\n"
 "    def initialize(name = \"\", volume = 100, pitch = 100)\n"
 "      super(name, volume, pitch)\n"
 "    end\n"
 "    @@last = BGM.new\n"
 "  end\n"
 "  class BGS < AudioFile\n"
 "    attr_accessor :pos\n"
 "    def replay\n"
 "      play(@pos)\n"
 "    end\n"
 "    def play(pos=0)\n"
 "      if @name == \"\"\n"
 "        Audio.bgs_stop\n"
 "        @@last = BGS.new\n"
 "      else\n"
 "        Audio.bgs_play(\"Audio/BGS/#@name\", @volume, @pitch, pos)\n"
 "        @@last = self.clone\n"
 "      end\n"
 "    end\n"
 "    def self.last\n"
 "      @@last.pos = Audio.bgs_pos\n"
 "      @@last\n"
 "    end\n"
 "    def self.stop\n"
 "      Audio.bgs_stop\n"
 "      @@last = BGS.new\n"
 "    end\n"
 "    def self.fade(time)\n"
 "      Audio.bgs_fade(time)\n"
 "      @@last = BGS.new\n"
 "    end\n"
 "    def initialize(name = \"\", volume = 100, pitch = 100)\n"
 "      super(name, volume, pitch)\n"
 "    end\n"
 "    @@last = BGS.new\n"
 "  end\n"
 "  class ME < AudioFile\n"
 "    def play\n"
 "      if @name == \"\"\n"
 "        Audio.me_stop\n"
 "      else\n"
 "        Audio.me_play(\"Audio/ME/#@name\", @volume, @pitch)\n"
 "      end\n"
 "    end\n"
 "    def self.stop\n"
 "      Audio.me_stop\n"
 "    end\n"
 "    def self.fade(time)\n"
 "      Audio.me_fade(time)\n"
 "    end\n"
 "    def initialize(name = \"\", volume = 100, pitch = 100)\n"
 "      super(name, volume, pitch)\n"
 "    end\n"
 "  end\n"
 "  class SE < AudioFile\n"
 "    def play\n"
 "      if @name != \"\"\n"
 "        Audio.se_play(\"Audio/SE/#@name\", @volume, @pitch)\n"
 "      end\n"
 "    end\n"
 "    def self.stop\n"
 "      Audio.se_stop\n"
 "    end\n"
 "    def initialize(name = \"\", volume = 100, pitch = 100)\n"
 "      super(name, volume, pitch)\n"
 "    end\n"
 "  end\n"
 "end\n"
 "\n"
// TODO: implement fake DL and Win32API
 "module DL\n"
 "end\n"
 "\n"
 "def rgss_stop\n"
 "  loop { Graphics.update }\n"
 "end\n"
 "def msgbox(*args)\n"
 "  print *args\n"
 "end\n"
 "def msgbox_p(*args)\n"
 "  p *args\n"
 "end\n"
 "RGSS_VERSION = \"3.0.1\"\n"
 "def save_data(obj, filename)\n"
 "  File.open(filename.gsub(/\\\\/, \"/\"), \"wb\") do|f|\n"
 "    Marshal.dump(obj, f)\n"
 "  end\n"
 "end\n"
;
